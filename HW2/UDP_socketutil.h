
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


#ifndef UDP_SOCKUTIL
#define UDP_SOCKUTIL


int create_working_udp_socket_client(char *IP_str, char *Port_str, struct sockaddr_in &server_addr);
int create_working_udp_socket_server(char *Port_str, struct sockaddr_in &server_ip_port);
void reliable_receive_packet(int socket_fd, char* local_file_buffer, char* remote_receive_buffer, uint32_t seq_num, ssize_t num_read);
int select_routine(int socket_fd, fd_set &reading_fds);
#endif