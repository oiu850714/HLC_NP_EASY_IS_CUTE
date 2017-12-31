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

using std::string;
using std::cin;
using std::cout;
using std::vector;
using std::stringstream;
using std::max;

#include "defineStruct.h"
#include "defineConstant.h"
#include "functionUtil.h"

void print(vector<user> &users)
{
    cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
    cout << "#users: " << users.size() << "\n";
    for(auto &user: users)
    {
        cout << "username: " << user.name << "\n";
        cout << "filelist: ";
        for(auto &filename : user.filelist)
        {
            cout << filename << " ";
        }
        cout << "\n";
    }
    cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
}

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("./server <port>");
        return 1;
    }
    //create server socket
    cout << "packet len: "<< sizeof(transfer_packet) << "\n";
    connection_server main_listening_connection;
    main_listening_connection.listen(argv[1]);
    //main_listening_connection.listen_socket_fd = new_tcp_listening_nonblocking_socket(argv[1]);
    
    vector<user> users;
    //store all clients' information
    
    vector<connection_server> new_connections;
    vector<connection_server> main_connections;
    vector<connection_server> upload_connections;
    vector<connection_server> listen_file_connections;
    vector<connection_server> download_connections;


    while(true)
    {
        if(main_listening_connection.nonblocking_accept() >= 0)
        {
            cout << "new client!\n";
            cout << "accept fd: " << main_listening_connection.socket_fd << "\n";
            //new client login!
            //send greeting
            //add to "wait for username list"
            new_connections.push_back(main_listening_connection);
        }

        for(int i = 0; i < new_connections.size(); )
        {
            if(new_connections[i].read_from_socket() == 0)
            {
                cout << "client close connection.\n";
                new_connections[i].close_socket();
                new_connections.erase(new_connections.begin());
            }
            else if(new_connections[i].can_parse_packet())
            {
                cout << "request: " << new_connections[i].get_request_type() << "\n";
                cout << "username: " << new_connections[i].packet.username << "\n";
                new_connections[i].reset_buffer_and_fill_member_from_packet();
                switch(new_connections[i].get_request_type())
                {
                    case CLI_RQ_UPLOAD_CONNECTION:
                        cout << "this is /put session!\n";
                        upload_connections.push_back(new_connections[i]);
                        //push this connection to client's main session connection
                        break;
                    case CLI_RQ_USERNAME:
                        cout << "client passs username, this is main session!\n";
                        new_connections[i].initialize_filelist(users);
                        main_connections.push_back(new_connections[i]);
                        //push this connection to client's upload session connection
                        break;
                    default:
                        cout << "don't set request type appropriately!\n";
                        break;
                }
                new_connections.erase(new_connections.begin()+i);
            }
            else
            {
                i++;
            }
        }
        
        for(int i = 0; i < main_connections.size(); i++)
        {
            if(main_connections[i].get_filelist_size() > 0)
            {
                string filename = main_connections[i].pop_filename();
                main_connections[i].set_filename(filename);
                listen_file_connections.push_back(main_connections[i]);
                //這裡沒開檔，等確定 client 連上新開的 socket 再開
            }
        }
        //this loop let users download their files

        for(int i = 0; i < download_connections.size(); i++)
        {
            download_connections[i].write_to_socket();
            if(download_connections[i].is_write_completely_packet())
            {
                cout << "write payload len: " << download_connections[i].get_payload_len() << "\n";
                download_connections[i].reset_buffer_and_fill_member_from_packet();
                if(download_connections[i].get_request_type() != SERV_RQ_DOWNFIN)
                {
                    download_connections[i].read_payload_from_file();
                    if(download_connections[i].get_payload_len() == 0)
                    {
                        cout << "all file content has been sent to client!\n";
                        download_connections[i].set_request_type(SERV_RQ_DOWNFIN);
                    }
                    else
                    {
                        download_connections[i].set_request_type(SERV_RQ_DOWNLOADING);
                    }
                    i++;
                }
                else
                {
                    cout << "close download file!\n";
                    download_connections[i].close_file();
                    download_connections[i].close_socket();
                    download_connections.erase(download_connections.begin() + i);
                }
            }
        }

        for(int i = 0; i < upload_connections.size();)
        {
            if(upload_connections[i].read_from_socket() == 0)
            {
                cout << "client has closed upload session(and main session, but I don't know how to close it using current function)\n";
                upload_connections[i].close_socket();
                upload_connections[i].close_file();
                upload_connections.erase(upload_connections.begin() + i);
                continue;
            }
            else if(upload_connections[i].can_parse_packet())
            {
                upload_connections[i].reset_buffer_and_fill_member_from_packet();
                switch(upload_connections[i].get_request_type())
                {
                    case CLI_RQ_UPLOADFILE:
                        upload_connections[i].openfile_to_write_server();
                        cout << "server open file to deal with client's /put command\n";
                        break;
                    case CLI_RQ_UPLOADING:
                        if(upload_connections[i].file_is_close())
                        {
                            cout << "fp is close but client send file content!\n";
                        }
                        else
                        {
                            upload_connections[i].wrtie_payload_to_file();
                            cout << "write byte: " << upload_connections[i].get_payload_len() << "\n";
                        }
                        break;
                    case CLI_RQ_UPFIN:
                        if(upload_connections[i].file_is_close())
                        {
                            cout << "fp is close but client say he finish transferring file!\n";
                        }
                        else
                        {
                            cout << "server receive all client's file content, close file!\n";
                            //這時候要把這個新 put 的 file 丟到其他同 username 的 clients
                            for(int j = 0; j < main_connections.size(); j++)
                            {
                                if(upload_connections[i].get_username() == main_connections[j].get_username()
                                    && upload_connections[i].get_socket_fd() != main_connections[j].get_socket_fd())
                                {
                                    cout << "/put socket_fd: " << upload_connections[i].get_socket_fd() << "\n";
                                    cout << "main socket_fd: " << main_connections[j].get_socket_fd() << "\n";
                                    main_connections[j].append_file_to_list(upload_connections[i].get_filename());
                                }
                            }

                            //在來是更新 users
                            int username_in_users_flag = 0;
                            for(int j = 0; j < users.size(); j++)
                            {
                                if(upload_connections[i].get_username() == users[j].name);
                                {
                                    username_in_users_flag = 1;
                                    int has_same_filename_flag = 0;
                                    for(auto &filename: users[j].filelist)
                                    {
                                        if(upload_connections[i].get_filename() == filename)
                                        {
                                            has_same_filename_flag = 1;
                                            break;
                                        }
                                    }
                                    if(has_same_filename_flag == 0)
                                    {
                                        users[j].filelist.push_back(upload_connections[i].get_filename());
                                        break;
                                    }
                                }
                            }
                            if(username_in_users_flag == 0)
                            {
                                user new_user;
                                new_user.name = upload_connections[i].get_username();
                                new_user.filelist.push_back(upload_connections[i].get_filename());
                                users.push_back(new_user);
                            }
                            print(users);
                            upload_connections[i].close_file();
                            //關檔案
                        }
                        break;
                }
            }
            i++;
        }
        //this loop receive users' /put command

        for (int i = 0; i < listen_file_connections.size(); i++)
        {
            switch(listen_file_connections[i].get_download_socket_state())
            {
                case FTM_OPEN_LISTEN_SOCKET:
                    cout << "listen!\n";
                    listen_file_connections[i].listen(LISTEN_ARBITRARY_PORT);
                    cout << "WHAT?\n";
                    listen_file_connections[i].set_sockaddr_by_getsockname();
                    listen_file_connections[i].set_request_type(SERV_RQ_DOWNLOAD);
                    listen_file_connections[i].set_download_socket_state(FTM_WRITE_SOCKADDR_TO_CLI);
                    break;
                case FTM_WRITE_SOCKADDR_TO_CLI:
                    listen_file_connections[i].write_to_socket();
                    if(listen_file_connections[i].is_write_completely_packet())
                    {
                        listen_file_connections[i].reset_buffer_and_fill_member_from_packet();
                        listen_file_connections[i].set_download_socket_state(FTM_LISTEN_CLI);
                    }
                    break;
                case FTM_LISTEN_CLI:
                    if(listen_file_connections[i].nonblocking_accept() >= 0)
                    {
                        //close this particular listening socket
                        listen_file_connections[i].close_listening_socket();
                        listen_file_connections[i].openfile_to_read_server();

                        listen_file_connections[i].read_payload_from_file();
                        listen_file_connections[i].set_request_type(SERV_RQ_DOWNLOADING);
                        // 一樣是先把 payload 讀進來，這樣 download socket 那個 loop 比較好寫

                        download_connections.push_back(listen_file_connections[i]);
                        listen_file_connections.erase(listen_file_connections.begin() + i);

                        i--;
                        // 這個 i-- 跟前面把 element 刪掉是一樣的道理，只是如果這裡不寫 i--，那 i++ 就要分開寫在很多地方ㄌ
                    }
            }
        }
        /* 1. create listening socket
         * 2. accepting socket
         * 3. throw new accepting socket to download connections
         */ 
         
    }
}