#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>

class Socket
{
public:
    Socket();
    Socket(int domain, int type, int protocol);
    
    //Syscall Wrapper
    ////connect socket accept listen bind;
    int Connect(int s, const struct sockaddr *name, socklen_t namelen);
    int Accept(int s, struct sockaddr * restrict addr, socklen_t * restrict addrlen);
    int Listen(int s, int backlog);
    int Bind(int s, const struct sockaddr *addr, socklen_t addrlen);
    
    
    int setIP();
    int setPort();
    
}