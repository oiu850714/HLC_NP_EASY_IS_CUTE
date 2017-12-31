#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <string>
#include <sys/select.h>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstdio>

//int initialize_socket(char *ip_str, char *port_str,
/* value-result arg*/ //int *socket_fd, struct *sockaddr_in sock_struct);

using std::string;
using std::cin;
using std::cout;
using std::vector;
using std::stringstream;
using std::max;
using std::to_string;

#include "defineStruct.h"
#include "defineConstant.h"
#include "functionUtil.h"


int max_fd(vector<connection> &connections)
{
    int temp_fd = 0;
    //printf("temp_fd: %d\n", temp_fd);
    for(auto  connection: connections)
    {
        if(connection.socket_fd > temp_fd)
            temp_fd = connection.socket_fd;
    }
    return temp_fd;
}

int main(int argc, char **argv)
{
    if(argc < 4)
    {
        printf("./client <ip> <port> <username>");
        return 1;
    }

    string socket_string(argv[1]);
    string socket_port(argv[2]);
    string username(argv[3]);

    connection main_socket_connection;
    main_socket_connection.socket_fd = new_tcp_connection(argv[1], argv[2]);
    connection upload_file_connection;
    upload_file_connection.socket_fd = new_tcp_connection(argv[1], argv[2]);

    main_socket_connection.set_username(username);
    main_socket_connection.set_request_type(CLI_RQ_USERNAME);
    cout << "main request type: " << main_socket_connection.get_request_type() << "\n";

    upload_file_connection.set_username(username);
    upload_file_connection.set_request_type(CLI_RQ_UPLOAD_CONNECTION);
    cout << "upload request type: " << upload_file_connection.get_request_type() << "\n";
    
    while(true)
    {
        main_socket_connection.write_to_socket();
        if(main_socket_connection.is_write_completely_packet())
        {
            main_socket_connection.reset_buffer_and_fill_member_from_packet();
            break;
        }
    }

    while(true)
    {
        upload_file_connection.write_to_socket();
        if(upload_file_connection.is_write_completely_packet())
        {
            upload_file_connection.reset_buffer_and_fill_member_from_packet();
            break;
        }
    }


    cout << "Welcome to the dropbox-like server! : " << argv[3] << "\n";

    vector<connection> connections;
    char commandline[MAXLINE];

    while (true) 
    {
        fd_set reading_fds;
        fd_set writing_fds;
        struct timeval tv;
    
        tv.tv_sec = 0;
        tv.tv_usec = 500;
        FD_ZERO(&reading_fds);
        FD_SET(STDIN_FILENO, &reading_fds);
        FD_SET(main_socket_connection.socket_fd, &reading_fds);
        FD_SET(upload_file_connection.socket_fd, &writing_fds);

        for (auto file_connection : connections)
        {
            FD_SET(file_connection.socket_fd, &reading_fds);
        }
        int maxfdp = max(
                max(main_socket_connection.socket_fd, upload_file_connection.socket_fd), 
                max_fd(connections)) + 1;

        int new_command_flag = 0, server_reply_flag = 0;

        switch (select(maxfdp, &reading_fds, &writing_fds, NULL, &tv))
        {
            case -1:
                exit(-1);
                break;
            case 0:
                break;
            default:
                if(FD_ISSET(STDIN_FILENO, &reading_fds))
                {
                    new_command_flag = 1;
                    if(fgets(commandline, MAXLINE, stdin) == NULL)
                        exit(0);
                    // read commandline, but the content will be parsed later
                }
                if(FD_ISSET(main_socket_connection.socket_fd, &reading_fds))
                {
                    int n = main_socket_connection.read_from_socket();
                    if(n == 0)
                    {
                        cout << "server close connection.\n";
                        exit(0);
                    }
                    if(main_socket_connection.can_parse_packet())
                    {
                        // reset buffer pointer and read data length
                        main_socket_connection.reset_buffer_and_fill_member_from_packet();
                        
                        int32_t server_request = main_socket_connection.get_request_type();
                        
                        //read to main_socket's record
                        switch(server_request)
                        {
                            case SERV_RQ_DOWNLOAD:
                                connections.push_back(connection());
                                connections.back().set_sockaddr(main_socket_connection.get_sockaddr());
                                connections.back().set_filename(main_socket_connection.get_filename());
                                connections.back().openfile_to_write();
                                connections.back().connect_by_sockaddr_in();
                                break;
                        }
                    }
                }
                
                if(FD_ISSET(upload_file_connection.socket_fd, &writing_fds))
                {
                    if(!upload_file_connection.file_is_close())
                    {
                        upload_file_connection.write_to_socket();

                        if(upload_file_connection.is_write_completely_packet())
                        {
                            upload_file_connection.reset_buffer_and_fill_member_from_packet();
                            if(upload_file_connection.get_request_type() != CLI_RQ_UPFIN)
                            {   
                                upload_file_connection.read_payload_from_file();
                                cout << "read payload len: " << upload_file_connection.get_payload_len() << "\n";
                                // may read 0 byte, which means EOF(or fucking error), and payload_len will be set to 0
                                if(upload_file_connection.get_payload_len() == 0)
                                {
                                    cout << "file content all been read\n";
                                    upload_file_connection.set_request_type(CLI_RQ_UPFIN);
                                }
                                else
                                {
                                    upload_file_connection.set_request_type(CLI_RQ_UPLOADING);
                                }
                            }
                            else
                            {
                                cout << "close file\n";
                                upload_file_connection.close_file();
                            }
                        }
                    }
                }

                for(auto &file_connection : connections)
                {
                    if(FD_ISSET(file_connection.socket_fd, &reading_fds))
                    {
                        int n = file_connection.read_from_socket();
                        if(n == 0)
                        {
                            cout << "server close connection.\n";
                            exit(0);
                        }
                        if(file_connection.can_parse_packet())
                        {
                            //server send file content
                            file_connection.reset_buffer_and_fill_member_from_packet();
                            int32_t server_request = file_connection.get_request_type(); // "0", "1", ...
                            switch(server_request)
                            {
                                case SERV_RQ_DOWNLOADING:
                                    file_connection.wrtie_payload_to_file();
                                    break;
                                case SERV_RQ_DOWNFIN:
                                    file_connection.close_file();
                                    file_connection.close_socket();
                                    break;
                            }
                        }
                    }
                }
                for(int i = 0; i < connections.size();)
                {
                    if(connections[i].file_is_close())
                    {
                        connections.erase(connections.begin() + i);
                    }
                    else
                    {
                        i++;
                    }
                }
                break;
        }

        if(new_command_flag)
        {
            new_command_flag = 0;   
            stringstream SS;
            SS << commandline;
            string command;
            SS >> command;
            if(string("/put") == command)
            {
                string filename;
                SS >> filename;

                upload_file_connection.set_request_type(CLI_RQ_UPLOADFILE);
                upload_file_connection.set_filename(filename);
                upload_file_connection.set_username(username);
                upload_file_connection.openfile_to_read();
                upload_file_connection.write_to_socket();
                upload_file_connection.reset_buffer_and_fill_member_from_packet();
                //反正一 write 之後，object 內部指向 buffer 的指標就會被更動，不管怎樣都要 reset buffer

                //先讀第一段 file content，這樣上面的 FD_ISSET(upload_file_connection.socket_fd, &writing_fds) 比較好寫
                upload_file_connection.read_payload_from_file();
                upload_file_connection.set_request_type(CLI_RQ_UPLOADING);

                /*
                transfer_packet file_put_request_packet(CLI_RQ_UPLOADFILE, argv[3], filename.c_str(), NULL, 0, NULL);
                */
            }
            else if(string("/sleep") == command)
            {
                int sleep_time;
                SS >> sleep_time;
                int counter = 0;
                while(sleep_time--)
                {
                    counter++;
                    sleep(1);
                    cout << "Sleep " << counter << "\n";
                }
                cout << "Client wakes up\n";
            }
            else if(string("/exit") == command)
            {
                exit(0);
            }
        }
    }

}