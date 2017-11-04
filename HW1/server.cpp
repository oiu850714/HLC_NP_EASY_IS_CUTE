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

using std::string;
using std::vector;

struct client_info
{
    sockaddr_in client_ip_port;
    string name = "anonymous";
    int client_socket_fd;
}

int main(int argc, char **argv)
{
    if(arcg < 2)
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
    bzero(server_ip_port, sizeof(server_ip_port));
    server_ip_port.sin_family = AF_INET;
    server_ip_port.sin_port = atoi(argv[1]);
    server_ip_port.sin_addr.s_addr = htonl (INADDR_ANY);
    if(!bind(server_socket_fd, server_ip_port))
    {
        printf("bind socket error\n");
        exit(1);
    }

    //listen socket
    listen(server_socket_fd, 15);
    
    vector<client_info>;
    //store all clients' information

    //----------------select routine---------------
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 50000;
    
    fd_set reading_fds;
    //fd_set writing_fds;

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
    
    while(true)
    {
        sockaddr_in new_client_ip_port;
        bzero(&new_client_ip_port, sizeof(sockaddr_in));
        int new_client_fd = accept((sockaddr*)new_client_ip_port)
    }
}