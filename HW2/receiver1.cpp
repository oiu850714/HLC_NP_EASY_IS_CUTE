#include "UDP_socketutil.h"
#include "FILE_util.h"
#include <fstream>
#include <cstdio>
#include <sstream>
#include <iostream>

// 1500 -28 - sizeof(SYN + FIN + ACK + seq_num)


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
    //what it does: create, bind, arg_2 is reference


    sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t addrlen;
    //client_addr get client's info by passing to recvfrom


    ssize_t recv_num;
    FILE* fp_copy_to_receiver = NULL;

    uint32_t cur_seq_num = 0;
    struct reliable_packet recv_packet;
    while(recv_num = recvfrom(server_fd, &recv_packet, PACKET_SIZE, 0,(sockaddr *) &client_addr, &addrlen))
    {
        //recv_packet[recv_num] = '\0';
        //printf("%s\n", recv_packet);
        printf("received!\n");
        
        uint32_t SYN = ntohl(recv_packet.SYN);
        uint32_t ACK = ntohl(recv_packet.ACK);
        uint32_t FIN = ntohl(recv_packet.FIN);
        uint32_t seq_num = ntohl(recv_packet.seq_num);
        uint32_t payload_len = ntohl(recv_packet.payload_len);
        
        cout << "SYN: " << SYN << "\n";
        cout << "ACK: " << ACK << "\n";
        cout << "FIN: " << FIN << "\n";
        cout << "seq_num:" << seq_num << "\n";
        cout << "payload_len: " << payload_len << "\n";
        
        if(seq_num == 0 && SYN == 1 && fp_copy_to_receiver == NULL)// only open file once
        {
            //if fp_copy_to_receiver != NULL, that means first packet containing file name has been received
            //so don't open file again
            cout << "open file!!\n";
            fp_copy_to_receiver = Fopen(recv_packet.payload, "wb");
            //now recv_packet.payload is null terminated string, so fopen is safe
            //cur_seq_num += 1;
            cur_seq_num = 1;
            // new client, reset cur_seq_num = 1
        }
        else if(FIN == 1 && fp_copy_to_receiver != NULL)
        {
            fclose(fp_copy_to_receiver);
            fp_copy_to_receiver = NULL;
        }
        else if(cur_seq_num == seq_num)
        {   
            cout << "cur_seq_num: " << cur_seq_num << "\n";
            //cout << "content written to file:\n" << content; 
            fwrite(recv_packet.payload, 1, payload_len, fp_copy_to_receiver) ;
            cur_seq_num += 1;
        }
        
        struct reliable_packet 
            ack_packet(htonl(0), htonl(0), htonl(1), htonl((uint32_t)cur_seq_num));
        ack_packet.payload_len = htonl(0);
        ack_packet.payload[0] = '\0';
        sendto(server_fd, &ack_packet, PACKET_SIZE, 0, (sockaddr *) &client_addr, sizeof(client_addr));
    }
}