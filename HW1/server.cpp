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

//#include <Server.h>

#define MAXLINE 2000
#define SERVERCOUNT 1
#define FUCKYOU "fuckyou!!!!!!!!!!!"

using std::string;
using std::vector;

struct client_info
{
    sockaddr_in ip_port;
    string name = "anonymous";
    int socket_fd;
};

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

        //reading fds
        FD_ZERO(&reading_fds);
        FD_SET(server_socket_fd, &reading_fds);
        printf("set fd\n");
        for(int i = 0; i < clients.size(); i++)
        {
            FD_SET(clients[i].socket_fd, &reading_fds);
        }
        int maxfdp = clients.size() + SERVERCOUNT + 1;
        printf("set fd complete\n");

        printf("select:\n");
        switch( select(maxfdp, &reading_fds, NULL, NULL, NULL))
        {
            case -1:
                exit(-1);
                break;
            case 0:
                break;
            default:
                if(FD_ISSET(server_socket_fd, &reading_fds))// new connection from client
                {
                    printf("new client!!!\n");
                    client_info new_client;
                    bzero(&new_client.ip_port, sizeof(sockaddr_in));
                    socklen_t len;
                    int new_client_fd =
                        accept(server_socket_fd, (sockaddr*)&new_client.ip_port, &len);
                    write(new_client_fd, FUCKYOU, sizeof(FUCKYOU));
                    printf("add new client complete\n");
                }
                else
                {
                    char command_from_client[MAXLINE];
                    for(int i = 0; i < clients.size(); i++)
                    {
                        if(FD_ISSET(clients[i].socket_fd, &reading_fds)) // some client sends command to server
                        {
                            write(clients[i].socket_fd, FUCKYOU, sizeof(FUCKYOU));
                        }
                    }
                }           
        }
        printf("select complete\n");
    }
}