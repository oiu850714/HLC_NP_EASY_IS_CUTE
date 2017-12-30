

#ifndef FUNCTIONUTIL
#define FUNCTIONUTIL

#include <cstdio>
#include "includeHeader.h"

FILE* Fopen(const char *filename, const char* mode);
int new_tcp_connection(char *IP_str, char *port_str);
int use_sockaddr_in_to_connect(sockaddr_in &server_addr);
int Socket();
void set_to_nonblocking(int &socket_fd);
int new_tcp_listening_nonblocking_socket(const char *port_str);

#endif