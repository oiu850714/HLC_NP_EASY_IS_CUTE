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
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <cctype>

#include "stringConstant.h"
#include "defineStruct.h"
#include "functionUtil.h"

using std::string;
using std::vector;
using std::max;
//using std::to_string;
using std::stringstream;


int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("./server <SERVER_PORT>\n");
        return 1;
    }
    //create server socket
    int server_socket_fd;
    if( (server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("open socket failed.\n");
        exit(1);
    }

    //bind socket
    sockaddr_in server_ip_port;
    bzero(&server_ip_port, sizeof(server_ip_port));
    server_ip_port.sin_family = AF_INET;
    server_ip_port.sin_port = htons(atoi(argv[1]));
    server_ip_port.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(server_socket_fd, (sockaddr *)&server_ip_port, sizeof(server_ip_port)) == -1)
    {
        printf("bind socket error\n");
        exit(1);
    }

    //listen socket
    listen(server_socket_fd, 15);
    
    vector<client_info> clients;
    //store all clients' information
    
    while(true)
    {
        //----------------select routine---------------
        fd_set reading_fds;
        //fd_set writing_fds;

        struct timeval tv;
        
        tv.tv_sec = 1;
        tv.tv_usec = 500000;

        //reading fds
        FD_ZERO(&reading_fds);

        //add all clients' socketfd and server's socketfd to reading_fds
        FD_SET(server_socket_fd, &reading_fds);
        for(int i = 0; i < clients.size(); i++)
        {
            FD_SET(clients[i].socket_fd, &reading_fds);
        }
        
        int maxfdp = max(max_fd(clients), server_socket_fd);

        switch(select(maxfdp + 1, &reading_fds, NULL, NULL, &tv))
        {
            case -1:
                printf("select error\n");
                exit(1);
                break;
            case 0:
                printf("no client\n");
                break;
            default:
                printf("default\n");
                if(FD_ISSET(server_socket_fd, &reading_fds))// new connection from client
                {
                    printf("new client!!!\n");
                    int new_client_sockfd = send_welcome_and_add_client(server_socket_fd, clients);
                    printf("add new client complete\n");
                    brocast_to_others(new_client_sockfd, clients);
                }
                else
                {

                    char command_from_client[MAXLINE];
                    for(int i = 0; i < clients.size(); i++)
                    {
                        if(FD_ISSET(clients[i].socket_fd, &reading_fds)) // some client sends command to server
                        {
                            int n = read(clients[i].socket_fd, command_from_client, MAXLINE);
                            command_from_client[n] = 0;
                            if(n == 0)//this user is offline
                            {
                                clear_offline_user(clients, clients[i].socket_fd);
                                break;   
                            }
                            printf("client %d's command: %s", clients[i].socket_fd, command_from_client);
                            parse_command_and_send_message(clients, command_from_client, clients[i].socket_fd);
                        }
                    }
                } 
                break;          
        }
        printf("select complete\n");
    }
}