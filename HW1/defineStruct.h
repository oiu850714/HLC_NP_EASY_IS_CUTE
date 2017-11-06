#include <string>
#include <netinet/in.h>
using std::string;

#ifndef DEFINE_STRUCT
#define DEFINE_STRUCT
struct client_info
{
    sockaddr_in ip_port;
    string name = "anonymous";
    int socket_fd;
};
#endif