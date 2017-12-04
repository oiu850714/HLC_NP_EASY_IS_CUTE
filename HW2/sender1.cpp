#include "UDP_socketutil.h"
#include "FILE_util.h"
#include <fstream>
#include <cstdio>
#include <sstream>
#include <iostream>

// 1500 -28 - sizeof(SYN + FIN + ACK + seq_num)

#define MAX_UDP_SIZE 100
//int initialize_socket(char *ip_str, char *port_str,
/* value-result arg*/ //int *socket_fd, struct *sockaddr_in sock_struct);

using std::string;
using std::to_string;
using std::stringstream;
using std::cout;

int main(int argc, char ** argv)
{

    if (argc < 4)
    {
        printf("./client <SERVER_IP> <SERVER_PORT <File_Name>\n");
        return 1;
    }

    struct sockaddr_in server_addr;
    int socket_fd = create_working_udp_socket_client(argv[1], argv[2], server_addr);
    //what it does: create, translate command line IP and port, connect

    FILE *fp = Fopen(argv[3], "rb");
    //open file safely

    off_t file_size = get_file_size(fp);
    printf("file size: %ld\n", file_size);
    //get file size
    
    /*
    struct reliable_packet packet(htonl(1), htonl(0), htonl(0), htonl(0));
    //                            SYN,      not FIN,  not ACK,  seq_num = 0
    packet.payload_len = htonl( (uint32_t)strlen(argv[3]));
    memcpy(packet.payload, argv[3], strlen(argv[3]));
    write(socket_fd, &packet, PACKET_SIZE);
    */
    uint32_t seq_num = 0;

    /*
    while(true)
    {
        fd_set reading_fds;
        int recv_flag = 0;
        switch(select_routine(socket_fd, reading_fds))
        {
            case -1:
                printf("select error\n");
                exit(1);
                break;
            case 0:
                printf("socket timeout\n");
                //cout << "send packet's content:\n" << first_packet_content;
                write(socket_fd, &packet, PACKET_SIZE);
                break;
            default:
                if(FD_ISSET(socket_fd, &reading_fds))
                {
                    printf("first packet's reply!\n");
                    struct reliable_packet recv_packet;
                    int n = read(socket_fd, &recv_packet, sizeof(recv_packet));
                    if(ntohl(recv_packet.seq_num) > seq_num)
                    {
                        //it's important that receive_seq_num > seq_num
                        //that means receiver receives "latest" packet correctly
                        printf("other side receive latest packet!!\n");
                        recv_flag = 1;
                    }
                }
                break;
        }
        if(recv_flag)
        {
            seq_num += 1;
            break;
        }
    }
    */

    reliable_receive_packet_select(socket_fd, argv[3], seq_num, strlen(argv[3]), 1, 0);
    //first packet                                                      SYN FIN
    seq_num++;

    char local_file_buffer[MAXLINE];
    ssize_t num_read;
    while(num_read = fread(local_file_buffer, 1, MAXLINE, fp))
    {
        reliable_receive_packet_select(socket_fd, local_file_buffer, seq_num, num_read, 0, 0);
        seq_num++;
    }

    //struct reliable_packet last_packet(htonl(0), htonl(1), htonl(0), htonl(0));
    
    reliable_receive_packet_select(socket_fd, NULL, seq_num, 0, 0, 1);

    //write(socket_fd, reliable_packet, PACKET_SIZE);
    printf("write comp\n");
    fclose(fp);
}
