#include "UDP_socketutil.h"
#include "FILE_util.h"
#include <fstream>
#include <cstdio>
#include <sstream>
#include <iostream>

#define MAXLINE 2000
#define MAX_UDP_SIZE 100


using std::string;
using std::stringstream;
using std::cout;
using std::to_string;

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("./server <SERVER_PORT>\n");
        return 1;
    }

    struct sockaddr_in server_ip_port;
    int server_fd = create_working_udp_socket_server(argv[1], server_ip_port);
    //what it does: create, bind

    ssize_t recv_num;
    char local_file_buffer[MAXLINE];
    char remote_receive_buffer[MAXLINE];
    FILE* fp;

    sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t addrlen;

    uint32_t cur_seq_num = 0;
    while(recv_num = recvfrom(server_fd, local_file_buffer, MAXLINE, 0,(sockaddr *) &client_addr, &addrlen))
    {
        local_file_buffer[recv_num] = '\0';
        //printf("%s\n", local_file_buffer);
        printf("received!\n");
        stringstream SS;
        SS << local_file_buffer;
        int seq_num;
        char eat_newline;
        SS >> seq_num;
        SS.get(eat_newline);
        //SS >> eat_newline;

        string content;
        char just_char;
        while(SS.get(just_char))
        {
            if( !(seq_num == 0 && just_char == '\n')) // to avoid add newline in file name
                content += just_char;
        }
        cout << "seq_num: " << seq_num << "\n";
        cout << "content:\n" << content << "\n";
        if(seq_num == 0 && fp == NULL)// open file twice dosen't cause error wtf
        {
            //if fp != NULL, that means first packet containing file name has been received
            //so don't open file again
            fp = Fopen(content.c_str(), "w");
            cur_seq_num += 1;
        }
        else if(cur_seq_num == seq_num)
        {   
            cout << "cur_seq_num: " << cur_seq_num << "\n";
            cout << "content written to file:\n" << content; 
            fwrite(content.c_str(), 1, content.size(), fp);
            cur_seq_num += 1;
        }
        string ack_packet(to_string(cur_seq_num));
        ack_packet += "\n";
        sendto(server_fd, ack_packet.c_str(), ack_packet.size(), 0, (sockaddr *) &client_addr, sizeof(client_addr));
    }
}