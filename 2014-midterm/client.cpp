#include <iostream>
#include <sstream>

#include <sys/socket.h>
#include <sys/types.h>

#include <arpa/inet.h>

#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#include <cstring>

using std::cout;
using std::stringstream;


#define MAX_LINE 1500

int calc(int sockfd)
{

}

int main(int argc, char* argv [])
{
    if(argc < 3)
    {
        printf("Usage: %s <IP> <Port>.\n", argv[0]);
        return 0;
    }

    int servfd;
    if( (servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("open socket error.\n");
        return 1;
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr.s_addr) == 0)
    {
        printf("invalid IP address.\n");
        return 1;
    }
    

    if( connect (servfd,(sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    {
        printf("connect error.\n");
        return 1;   
    }

    
    while(1)
    {
        fd_set rset;
        FD_ZERO(&rset);
        FD_SET(servfd, &rset);
        FD_SET(0, &rset);

        int max_fd = servfd;

        select(max_fd + 1, &rset, NULL, NULL, NULL);

        char input_line[MAX_LINE];
        char result[MAX_LINE];
        if(FD_ISSET(0, &rset))
        {
            int n = read(0, input_line, MAX_LINE);
            if(n == 0)
            {
                printf("Bye.\n");
                exit(0);
            }

            input_line[n] = '\0';
            write(servfd, input_line, strlen(input_line));
        }
        if(FD_ISSET(servfd, &rset)  )
        {
            int n = read(servfd, result, MAX_LINE);
            if(n == 0)
            {
                printf("The server has closed the connection.\n");
                exit(0);
            }

            result[n] = '\0';
            printf("%s", result);
        }
    }
    
}