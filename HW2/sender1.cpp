#include "UDP_socketutil.h"
#include "FILE_util.h"
#include <fstream>
#include <cstdio>
#include <sstream>
#include <iostream>

#define MAXLINE 2004
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
    //FILE *fp_copy = Fopen("copy.txt", "w");
    ssize_t num_read;

    off_t file_size = get_file_size(fp);
    
    printf("file size: %ld\n", file_size);
    char local_file_buffer[MAXLINE];
    char sendto_remote_buffer[MAXLINE];
    char receive_remote_buffer[MAXLINE];
    uint32_t seq_num = 0;

    string first_packet_content;
    first_packet_content += to_string(seq_num) + "\n" + "0" + "\n" + argv[3] + "\n";
    write(socket_fd, first_packet_content.c_str(), first_packet_content.size());
    cout << "send packet's content:\n" << first_packet_content;
    //reliable_receive_packet(socket_fd, local_file_buffer, sendto_remote_buffer, seq_num, num_read);
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
                cout << "send packet's content:\n" << first_packet_content;
                write(socket_fd, first_packet_content.c_str(), first_packet_content.size());
                break;
            default:
                if(FD_ISSET(socket_fd, &reading_fds))
                {
                    printf("first packet's reply!\n");
                    int n = read(socket_fd, receive_remote_buffer, MAXLINE);
                    stringstream SS;
                    SS << receive_remote_buffer;
                    int receive_seq_num;
                    SS >> receive_seq_num;
                    if (receive_seq_num > seq_num)
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

    while(num_read = fread(local_file_buffer, 1, MAX_UDP_SIZE, fp))
    {
        //local_file_buffer[num_read] = '\0';
        //fwrite(local_file_buffer, 1, num_read, fp_copy);
        //----------------select routine---------------
        /*
        while(true)
        {   
            write(socket_fd, local_file_buffer, num_read);
            switch(select_routine(socket_fd))
            {
                -1:
                    printf("select error\n");
                    exit(1)
                    break;
                0:
                    printf("socket timeout\n");
                    break;
                default:
                    if(FD_ISSET(socket_fd, &reading_fds))
                    {
                        printf("received other side's reply!\n");
                        
                    }
            }
        }
        */
        reliable_receive_packet(socket_fd, local_file_buffer, sendto_remote_buffer, seq_num, num_read);
        seq_num++;
    }

    write(socket_fd, local_file_buffer, 0);
    printf("write comp\n");
    fclose(fp);
    //fclose(fp_copy);
}
