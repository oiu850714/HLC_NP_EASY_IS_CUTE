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
    string name;
    int socket_fd;
}