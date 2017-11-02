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

#define MAXLINE 2000
//int initialize_socket(char *ip_str, char *port_str,
/* value-result arg*/ //int *socket_fd, struct *sockaddr_in sock_struct);

using std::string;

int main(int argc, char ** argv)
{

    if(argc < 3)
    {
        printf("./client <SERVER_IP> <SERVER_PORT\n");
        return 1;
    }
    // inet_pton and inet_ntop need
    //bind, connect, sendto, and sendmsg,
    
    int socket_fd;
    if( (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("open socket failed.\n");
        exit(1);
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port =  htons(atoi(argv[2]));

    if(!inet_pton(AF_INET, argv[1], &server_addr.sin_addr))
    {
        printf("fuckyou invalid IP address\n");
        exit(1);
    }

    if (connect(socket_fd, (sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        printf("connect error\n");
        exit(1);
    }

    int n;
    char recvline[MAXLINE];
    recvline[0] = 0;
    char commandline[MAXLINE];
    commandline[0] = 0;
    while (true) 
    {
        //----------------select routine---------------
        fd_set reading_fds;
        fd_set writing_fds;
        struct timeval tv;
    
        tv.tv_sec = 0;
        tv.tv_usec = 50000;
        
        //reading fds
        FD_ZERO(&reading_fds);
        FD_SET(socket_fd, &reading_fds);
        FD_SET(0, &reading_fds);
        //no need to check writing fds, just fuck up them

        // maxfdp need to one more than all file_descriptor's max's value
        int maxfdp = socket_fd + 1;
        //above maybe need to fix?
        //maybe change the way maxfdp is assigned
        //e.g.
        //fd = socket(...);
        //maxfdp = maxfdp > fd ? maxfdp : fd;
        //--------------end of select routine-------------
        switch(select(maxfdp, &reading_fds, NULL, NULL, &tv))
        {
            case -1:
                exit(-1);
                break;
            case 0:
                break;
            default:
                if(FD_ISSET(0, &reading_fds))
                {
                    if(fgets(commandline, MAXLINE, stdin) == NULL)
                        exit(0);
                    if(string("exit\n") == commandline)
                        exit(0);
                    write(socket_fd, commandline, strlen(commandline));
                }
                if(FD_ISSET(socket_fd, &reading_fds))
                {
                    if( (n = read(socket_fd, recvline, MAXLINE)) > 0)
                    {
                        recvline[n] = 0;        // null terminate 
                        if (fputs(recvline, stdout) == EOF)
                        {
                            printf("your terminal fucked up.\n");
                            return 0;
                        }
                    }
                }
                break;
        }
        FD_CLR(socket_fd, &reading_fds);
        FD_CLR(0, &reading_fds);
    }
}
