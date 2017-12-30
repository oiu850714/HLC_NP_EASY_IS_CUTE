#include "functionUtil.h"
#include <cstdio>
#include <iostream>

using std::cout;

int Socket()
{
    int socket_fd;
    if( (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("open socket failed.\n");
        exit(1);
    }
    return socket_fd;
}


FILE* Fopen(const char *filename, const char* mode)
{
    FILE* fp = fopen(filename, mode);
    if(fp == NULL)
    {
        cout << "open file failed\n";
        exit(1);
    }
    return fp;
}

int new_tcp_connection(char *IP_str, char *port_str)
{
    int socket_fd = Socket();
    /*
    if( (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("open socket failed.\n");
        exit(1);
    }
    */

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port =  htons(atoi(port_str));

    if(!inet_pton(AF_INET, IP_str, &server_addr.sin_addr))
    {
        printf("fuckyou invalid IP address\n");
        exit(1);
    }

    if (connect(socket_fd, (sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        printf("connect error\n");
        exit(1);
    }
    return socket_fd;
}

int use_sockaddr_in_to_connect(sockaddr_in &server_addr)
{
    int socket_fd = Socket();

    if (connect(socket_fd, (sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        printf("connect error\n");
        exit(1);
    }
    return socket_fd;
}

void set_to_nonblocking(int &socket_fd)
{
    int fcntl_val = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, fcntl_val | O_NONBLOCK);
    // set socket to nonblocking
}


int new_tcp_listening_nonblocking_socket(const char *port_str)
{
    int server_socket_fd;
    if( (server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("open socket failed.\n");
        exit(1);
    }

    //bind socket
    sockaddr_in server_ip_port;
    //bzero(&server_ip_port, sizeof(server_ip_port));
    memset(&server_ip_port, 0, sizeof(server_ip_port));
    server_ip_port.sin_family = AF_INET;
    server_ip_port.sin_port = htons(atoi(port_str));
    server_ip_port.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(server_socket_fd, (sockaddr *)&server_ip_port, sizeof(server_ip_port)) == -1)
    {
        printf("bind socket error\n");
        exit(1);
    }

    set_to_nonblocking(server_socket_fd);
    //先關掉該死的nonblocking
    //set to nonblocking

    //listen socket
    listen(server_socket_fd, 100);

    return server_socket_fd;
}

