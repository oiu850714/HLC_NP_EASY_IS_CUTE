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

using std::string;

struct client_info
{
    sockaddr_in client_ip_port;
    string name = "anonymous";
    int client_socket_fd;
}

int main()
{
    //create server socket
    int server_socket_fd;
    if( (server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("open socket failed.\n");
        exit(1);
    }

    //bind socket
    sockaddr_in server_ip_port;
    bzero(server_ip_port, sizeof(server_ip_port));
    server_ip_port.sin_family = AF_INET;
    server_ip_port.sin_port = 
    if(!bind(server_socket_fd, ))
}