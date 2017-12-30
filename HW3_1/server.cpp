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


void find_user_and_add_filelist(connection_server &new_connection, vector<user> &users)
{
    for(auto &user : users)
    {
        if(user.name == new_connection.username)
        {
            new_connection.filelist = user.filelist;
            return;
        }
    }
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

        cout << "new_connections.size(): " << new_connections.size() << "\n";
        for(int i = 0; i < new_connections.size(); )
        {
            new_connections[i].read_from_socket();
            cout << "fd that would block: " << new_connections[i].socket_fd << "\n";
            if(new_connections[i].can_parse_packet())
            {
                cout << "request: " << new_connections[i].get_request_type() << "\n";
                cout << "username: " << new_connections[i].packet.username << "\n";
                new_connections[i].reset_buffer();
                switch(new_connections[i].get_request_type())
                {
                    case CLI_RQ_UPLOAD_CONNECTION:
                        cout << "this is /put session!\n";
                        break;
                    case CLI_RQ_USERNAME:
                        cout << "client passs username!\n";
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
        sleep(1);
    }
}
/*
int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("./server <port>");
        return 1;
    }
    //create server socket
    int server_socket_fd = new_tcp_listening_nonblocking_socket(argv[1]);
    
    vector<user> users;
    //store all clients' information
    
    vector<connection_server> connections_wait_username;
    vector<connection_server> connections_main_socket;
    vector<connection_server> connections_listen_file_transfer;
    vector<connection_server> connections_download_socket;
    vector<connection_server> connections_upload_socket;

    while(true)
    {
        int new_connection_fd = accept(server_socket_fd, NULL, NULL);
        if(new_connection_fd < 0)
        {
            if(errno != EWOULDBLOCK)
            {
                cout << "errno: " << errno << "\n";
                cout << "accepting error\n";
                exit(1);
            }
        }
        else
        {
            cout << "new client!\n";
            set_to_nonblocking(new_connection_fd);
            connection_server new_connection;
            new_connection.socket_fd = new_connection_fd;
            connections_wait_username.push_back(new_connection);
            //new client login!
            //send greeting
            //add to "wait for username list"
        }

        //dealing with "wait for username list"
        //if username is received, find corresponding user object, and put this client to "user main socket list"
        for(int i = 0; i < connections_wait_username.size();)
        {
            char username[USERNAMELEN];
            char buffer[MAXLINE];
            int n = read(connections_wait_username[i].socket_fd, buffer, MAXLINE);
            if (n < 0)
            {
                if(errno != EWOULDBLOCK)
                {
                    cout << "getting username error\n";
                    exit(1);
                }
                i++;
            }
            else
            {
                strncpy(username, buffer+1, USERNAMELEN); // +1 是因為第一個 byte 是 request type
                connections_wait_username[i].username = username;
                find_user_and_add_filelist(connections_wait_username[i], users);
                
                connections_main_socket.push_back(connections_wait_username[i]);
                //都弄完之後就加到 connections_main_socket 裡面

                connections_wait_username.erase(connections_wait_username.begin()+i);
                //然後把這個連線從 wait list 刪掉
            }
        }


        //dealing with "user main socket list"
        //if there is file to be download, put user to "download file list", this need "wrtie" to socket
        //need to write sockaddr_in to client and let it connect back
        // 流程應該是，從 filelist 抓一個檔名出來，然後創一個 listening socket(bind 任意port)，把這個 listening socket 丟到另一個 listening socket object list
        // 然後用 getsockname 把完整的 socketaddr_in 拿回來，然後把 sockaddr_in 用 main_socket 傳給 client。
        // 之後 listening socket list 就一直等 client connect，return 的 socket 直接丟到 "download file list"。

        /////////////////////////////////////////////////
        /*
         *
         * 
         * ALL nonblocking read write is shitty currently
         * 
         * 
         * 
         * //////////////////////////////////////////////
        
        for(int i = 0; i < connections_main_socket.size(); i++)
        {
            if(connections_main_socket[i].filelist.size() > 0)
            {
                connection_server new_listening_socket;
                new_listening_socket.username = connections_main_socket[i].username;
                new_listening_socket.filename = connections_main_socket[i].filelist[connections_main_socket[i].filelist.size()-1];
                //拿 filelist 的最後一個 filename 出來
                new_listening_socket.socket_fd =  new_tcp_listening_nonblocking_socket("0"); // "0" let OS select arbitrary port
                socklen_t socklen = sizeof(new_listening_socket.ip_port);
                getsockname(new_listening_socket.socket_fd, (sockaddr *) &(new_listening_socket.ip_port), &socklen);
                
                char buffer[MAXLINE];
                buffer[0] = SERV_RQ_DOWNLOAD;
                memcpy(buffer+1, &(new_listening_socket.ip_port), sizeof(new_listening_socket.ip_port));
                strncpy(buffer+1+sizeof(new_listening_socket.ip_port), new_listening_socket.filename.c_str(), new_listening_socket.filename.size());
                // buffer's content: request_type + sockaddr_in + filename
                int n = write(new_listening_socket.socket_fd, buffer, MAXLINE);
                if (n < 0)
                {
                    if(errno != EWOULDBLOCK)
                    {
                        cout << "file download request error\n";
                        exit(1);
                    }
                }
                else
                {
                    new_listening_socket.read_write_flag = READ_LOCAL_FILE_FLAG;
                    //when accept, put this object to download list
                    connections_listen_file_transfer.push_back(new_listening_socket);
                    connections_main_socket[i].filelist.pop_back();
                }
            }
        }

        //if there is an file to be upload, put user to "upload file list", this need "read" to socket
        for(int i = 0; i < connections_main_socket.size(); i++)
        {
            char read_buffer[MAXLINE];
            int n = read(connections_wait_username[i].socket_fd, read_buffer, MAXLINE);
            if (n < 0)
            {
                if(errno != EWOULDBLOCK)
                {
                    cout << "getting /put command error\n";
                    exit(1);
                }
            }
            else
            {
                // now buffer's content: request_type + filename
                char request_type = read_buffer[0];
                if(request_type == CLI_RQ_UPLOADFILE)
                {
                    connection_server new_listening_socket;
                    new_listening_socket.filename = read_buffer+1; // now buffer+1 is payload, which is filename user want to put
                    new_listening_socket.username = connections_main_socket[i].username;
                    new_listening_socket.socket_fd =  new_tcp_listening_nonblocking_socket("0"); // "0" let OS select arbitrary port
                    socklen_t socklen = sizeof(new_listening_socket.ip_port);
                    getsockname(new_listening_socket.socket_fd, (sockaddr *) &(new_listening_socket.ip_port), &socklen);
                    
                    char write_buffer[MAXLINE];
                    write_buffer[0] = SERV_RQ_UPLOAD;
                    memcpy(write_buffer+1, &(new_listening_socket.ip_port), sizeof(new_listening_socket.ip_port));
                    
                    int write_len = write(new_listening_socket.socket_fd, write_buffer, MAXLINE);
                    if (write_len < 0)
                    {
                        if(errno != EWOULDBLOCK)
                        {
                            cout << "file upload request error\n";
                            exit(1);
                        }
                    }
                    else
                    {
                        new_listening_socket.read_write_flag = WRITE_LOCAL_FILE_FLAG;
                        //when accept, put this object to upload list
                        connections_listen_file_transfer.push_back(new_listening_socket);
                    }
                }
            }
        }

        //when accept, throw socket to download list or upload list
        for(int i = 0; i < connections_listen_file_transfer.size(); i++)
        {
            int new_filetransfer_socket_fd = accept(connections_listen_file_transfer[i].socket_fd, NULL, NULL);
            if (new_filetransfer_socket_fd < 0)
            {
                if (errno != EWOULDBLOCK)
                {
                    cout << "accepting new filetransfer socket error\n";
                    exit(1);
                }
                i++;
            }
            else
            {
                connection_server new_accept_file_transer = connections_listen_file_transfer[i];
                new_accept_file_transer.socket_fd = new_filetransfer_socket_fd;
                // change listening socket to accepting socket
                
                //在這裡openfile 如何?
                if(connections_listen_file_transfer[i].read_write_flag == READ_LOCAL_FILE_FLAG)
                {
                    //throw it to download list
                    new_accept_file_transer.fp = Fopen(new_accept_file_transer.filename.c_str(), "rb");
                    connections_download_socket.push_back(new_accept_file_transer);
                    
                }  
                else
                {
                    new_accept_file_transer.fp = Fopen(new_accept_file_transer.filename.c_str(), "wb");
                    connections_upload_socket.push_back(new_accept_file_transer);
                    //throw it to upload list
                }
                close(connections_listen_file_transfer[i].socket_fd);
                connections_listen_file_transfer.erase(connections_listen_file_transfer.begin()+i);
            }
        }


        //dealing with "download file list"
        //if there is file to be download, keep in this list
        //else remove this user
        int send_flag = 0;
        for(int i = 0; i < connections_download_socket.size(); i++)
        {
            
            char file_content[MAXLINE];
            int read_len;
            int write_len;
            if(send_flag)
            {   
                if( (read_len = fread(file_content, 1, MAXLINE, connections_download_socket[i].fp)) == 0)
                {
                    fclose(connections_download_socket[i].fp);
                    connections_download_socket[i].fp = NULL;
                }
            }
            write_len = write(connections_download_socket[i].socket_fd, file_content, read_len);
            if(write_len < 0)
            {
                if(errno != EWOULDBLOCK)
                {
                    cout << "send file content to client error.";
                    exit(1);
                }
                send_flag = 0;
            }
            else
            {
                send_flag = 1;
            }
            if(connections_download_socket[i].fp == NULL)
            {
                connections_download_socket.erase(connections_download_socket.begin()+i);
            }
            else
            {
                i++;
            }
        }

        //below and above fread and fwrite is very shitty!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        //dealing with "upload file list"
        //if file uploading finish, check other clients with same username, add them to "download file list"
        int receive_flag = 0;
        for(int i = 0; i < connections_upload_socket.size(); i++)
        {
            char file_content[MAXLINE];
            int read_len;
            int write_len;
            if ( (read_len = read(connections_upload_socket[i].socket_fd, file_content, MAXLINE)) > 0)
            {
                fwrite(file_content, 1, read_len, connections_upload_socket[i].fp);
                receive_flag = 0;
            }
            else if (read_len == 0)
            {
                fclose(connections_upload_socket[i].fp);
                connections_upload_socket[i].fp = NULL;
                
                for(int j = 0; j < connections_main_socket.size(); j++)
                {
                    if(connections_main_socket[j].username == connections_upload_socket[i].username)
                    {
                        connections_main_socket[j].filelist.push_back(connections_upload_socket[j].filename);
                        // let main_socket routine handle new downloading request
                    }
                }
            }
        }
    }
}
*/