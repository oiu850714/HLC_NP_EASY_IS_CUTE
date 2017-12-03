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
#include <netdb.h>
#include <sstream>
#include <iostream>

#include "UDP_socketutil.h"

using std::stringstream;
using std::to_string;
using std::string;
using std::cout;

#define MAXLINE 2000

int Socket(int family, int sock_type, int protocol)
{
    int socket_fd;
    if( (socket_fd = socket(family, sock_type, protocol)) == -1)
    {
        printf("open socket failed.\n");
        exit(1);
    }
    return socket_fd;
}

void initialize_sock_struct_and_port(struct sockaddr_in &addr, char *&Port_str)
{
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(Port_str));
}

int create_working_udp_socket_server(char *Port_str, struct sockaddr_in &server_ip_port)
{
    int socket_fd = Socket(AF_INET, SOCK_DGRAM, 0);
    initialize_sock_struct_and_port(server_ip_port, Port_str);

    server_ip_port.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(socket_fd, (sockaddr *)&server_ip_port, sizeof(server_ip_port)) == -1)
    {
        printf("bind socket error\n");
        exit(1);
    }

    return socket_fd;   
}

int create_working_udp_socket_client(char *IP_str, char *Port_str, struct sockaddr_in &server_addr)
{
    int socket_fd = Socket(AF_INET, SOCK_DGRAM, 0);
    initialize_sock_struct_and_port(server_addr, Port_str);
        
    if(!inet_pton(AF_INET, IP_str, &server_addr.sin_addr.s_addr))
    {
        printf("invalid IP address\n");
        exit(1);
    }
    
    //this is UDP connectionless connect XDD, 
    if (connect(socket_fd, (sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        printf("connect error\n");
        exit(1);
    }

    return socket_fd;
}


int select_routine(int socket_fd, fd_set &reading_fds)
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000;
    
    //reading fds
    FD_ZERO(&reading_fds);
    FD_SET(socket_fd, &reading_fds);
    //no need to check writing fds, just fuck up them

    // maxfdp need to one more than all file_descriptor's max's value
    int maxfdp = socket_fd + 1;
    
    return select(maxfdp, &reading_fds, NULL, NULL, &tv);
}


void reliable_receive_packet(int socket_fd, char* local_file_buffer, char* remote_receive_buffer, uint32_t seq_num, ssize_t num_read)
{
    local_file_buffer[num_read] = '\0';
    string add_seq_payload(to_string(seq_num));
    add_seq_payload += "\n";
    add_seq_payload += local_file_buffer;
    cout << "send packet's content:\n" << add_seq_payload;
    write(socket_fd, add_seq_payload.c_str(), add_seq_payload.size());
    while(true)
    {
        fd_set reading_fds;
        switch(select_routine(socket_fd, reading_fds))
        {
            case -1:
                printf("select error\n");
                exit(1);
                break;
            case 0:
                printf("socket timeout\n");
                cout << "send packet's content:\n" << add_seq_payload;
                write(socket_fd, add_seq_payload.c_str(), add_seq_payload.size());
                break;
            default:
                if(FD_ISSET(socket_fd, &reading_fds))
                {
                    printf("received other side's reply!\n");
                    int n = read(socket_fd, local_file_buffer, MAXLINE);
                    stringstream SS;
                    SS << local_file_buffer;
                    int receive_seq_num;
                    SS >> receive_seq_num;
                    if (receive_seq_num > seq_num)
                    {
                        //it's important that receive_seq_num > seq_num
                        //that means receiver receives "latest" packet correctly
                        printf("other side receive latest packet!!\n");
                        return;
                    }
                }
                break;
        }
    }
}