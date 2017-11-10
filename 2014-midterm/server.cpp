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
#include <vector>
#include <sstream>
#include <iostream>

using std::vector;
using std::stringstream;
using std::string;

#define MAX_LINE 2000

int calc_or_close_fd(int sockfd)
{
    //return -1 in case that client type EOF or EXIT
    char client_cmd[MAX_LINE];
    int n = read(sockfd, client_cmd, MAX_LINE);
    if(n == 0)
    {
        close(sockfd);
        return -1;
    }

    client_cmd[n] = '\0';
    stringstream SS;
    SS << client_cmd;
    std::cout << "client_cmd: " << SS.str();

    if(SS.str() == "\n")
        return 0;
    
    string cmd;
    SS >> cmd;
    int num_operand;
    
    int result;
    char operand[20];
    if(cmd == "ADD")
    {
        result = 0;
        printf("ADD:\n");
        SS >> num_operand;
        while(num_operand--)
        {
            SS >> operand;
            result += atoi(operand);
            printf("temp result: %d\n", result);
        }

    }
    else if(cmd == "MUL")
    {
        result = 1;
        printf("MUL:\n");
        SS >> num_operand;
        while(num_operand--)
        {
            SS >> operand;
            result *= atoi(operand);
            printf("temp result: %d\n", result);
        }
    }
    else if(cmd == "EXIT")
    {
        close(sockfd);
        return -1;
    }
    else
    {
        write(sockfd, "Error OPCODE\n", strlen("Error OPCODE\n"));
        return 0;
    }
    char result_str[MAX_LINE];
    snprintf(result_str, MAX_LINE, "%d\n", result);
    printf("result_str: %s", result_str);
    write(sockfd, result_str, strlen(result_str));
    return 0;
}

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        printf("Usage: %s <port>.\n", argv[0]);
        return 0;
    }
    
    int server_listen_fd;
    if( (server_listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("open socket error.\n");
        return 1;
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[1]));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(server_listen_fd, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1)
    {
        printf("bind socket error.\n");
        return 1;
    }

    listen(server_listen_fd, 15);

    vector<int> client_fds;

    //select
    while(1)
    {
        fd_set rset;
        FD_ZERO(&rset);
        FD_SET(server_listen_fd, &rset);
        
        int max_fd = server_listen_fd;
        for(auto fd : client_fds)
        {
            FD_SET(fd, &rset);
            max_fd = fd > max_fd ? fd : max_fd;
        }
        //add listening socket and all connected sockets to rset

        struct timeval tv;
        
        tv.tv_sec = 1;
        tv.tv_usec = 500000;

        int num_ready_fd = select(max_fd + 1, &rset, NULL, NULL, &tv);
        printf("select complete, num_ready_fd: %d\n", num_ready_fd);

        if(num_ready_fd == -1)
        {
            printf("select error.\n");
            exit(1);
        }

        if(FD_ISSET(server_listen_fd, &rset))
        {
            printf("add new client!\n");
            struct sockaddr_in client_addr;
            memset(&client_addr, 0, sizeof(client_addr));
            socklen_t sock_len = sizeof(client_addr);
            int new_cli_fd = accept(server_listen_fd, (struct sockaddr *) &client_addr, &sock_len);
            client_fds.push_back(new_cli_fd);
            //accept, add connected socket to client_fds
        }

        printf("outside for\n");
        for(int i = 0; i < client_fds.size(); i++)
        {
            if(FD_ISSET(client_fds[i], &rset))
            {
                char client_cmd[MAX_LINE];
                if(calc_or_close_fd(client_fds[i]) == -1)
                {
                    printf("YOOOOOOOOOO\n");
                    client_fds.erase(client_fds.begin() + i);
                }
                if(--num_ready_fd == 0)
                        break;
            }
        }
        printf("outside for complete\n");
    }
}