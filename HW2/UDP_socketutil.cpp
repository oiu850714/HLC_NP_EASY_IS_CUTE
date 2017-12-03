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

// 1500 -28 - sizeof(SYN + FIN + ACK + seq_num)



reliable_packet::reliable_packet()
{
    FIN = 0; SYN = 0; ACK = 0; seq_num = 0; payload_len = 0;
}

reliable_packet::reliable_packet(uint32_t SYN, uint32_t FIN, uint32_t ACK, uint32_t seq_num)
{
    this->FIN = FIN; this->SYN = SYN; this->ACK = ACK; this->seq_num = seq_num;
}

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


void reliable_receive_packet(int socket_fd, char* local_file_buffer, uint32_t seq_num, ssize_t num_read, uint32_t SYN, uint32_t FIN)
{
    struct reliable_packet send_packet;

    if(SYN == 1)
        send_packet = reliable_packet(htonl(1), htonl(0), htonl(0), htonl(seq_num));
    else if(FIN == 1)                //  SYN       FIN       ACK
        send_packet = reliable_packet(htonl(0), htonl(1), htonl(0), htonl(seq_num));
    else
        send_packet = reliable_packet(htonl(0), htonl(0), htonl(0), htonl(seq_num));
    
    send_packet.payload_len = htonl( (uint32_t)num_read);
    if(FIN == 0)
        memcpy(send_packet.payload, local_file_buffer, num_read);

    write(socket_fd, &send_packet, PACKET_SIZE);
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
                //cout << "send packet's content:\n" << whole_packet;
                write(socket_fd, &send_packet, PACKET_SIZE);
                break;
            default:
                if(FD_ISSET(socket_fd, &reading_fds))
                {
                    printf("received other side's reply!\n");
                    struct reliable_packet recv_packet;
                    int n = read(socket_fd, &recv_packet, PACKET_SIZE);

                    if (ntohl(recv_packet.seq_num) > seq_num)
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