both use reliable_packet to communicate:

struct reliable_packet
{
    uint32_t SYN;
    uint32_t FIN;
    uint32_t ACK;
    uint32_t seq_num;
    uint32_t payload_len;
    char payload[MAXLINE];

    reliable_packet();
    reliable_packet(uint32_t SYN, uint32_t FIN, uint32_t ACK, uint32_t seq_num);
};


cli: 
packet 0:
    SYN = 1, FIN = 0, ACK = 0, seq_num = 0, payload_len = strlen(file_name), payload = file_name
packet i from 1 to (file size)/best_udp_seg_size + 1:
    SYN = 0, FIN = 0, ACK = 0, seq_num = i, payload_len = num_read, payload = file content
last packet:
    SYN = 0, FIN = 1, ACK = 0, seq_num = last i, payload_len = 0, payload = empty


serv:
packet 0:
    SYN = 1, FIN = 0, ACK = 1, seq_num = 1, payload_len = 0, payload = empty
packet i from 1 to (file size)/best_udp_seg_size + 1:
    SYN = 0, FIN = 0, ACK = 1, seq_num = i + 1, payload_len = 0, payload = empty
last packet:
    SYN = 0, FIN = 0, ACK = 1, seq_num = i + 1, rest = 0

timeout:
    stop_and_wait LOL
