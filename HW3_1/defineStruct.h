#ifndef DEFINE_STRUCT
#define DEFINE_STRUCT

#define READ_LOCAL_FILE_FLAG 0
#define WRITE_LOCAL_FILE_FLAG 1

#include "includeHeader.h"
#include "defineConstant.h"
#include "functionUtil.h"

struct transfer_packet
{
    int32_t request_type;
    char username[50];
    char filename[50];
    sockaddr_in ip_port;
    int32_t payload_len;
    char payload[TRANSFER_PACK_PAYLOADLEN];


    transfer_packet(int32_t request_rype, const char* username, const char* filename, sockaddr_in* ip_port_ptr, int32_t payload_len, char* payload)
    {
        this->request_type = htonl(request_type);
        strncpy(this->username, username, strlen(username));
        strncpy(this->filename, filename, strlen(filename));
        if(ip_port_ptr != NULL)
            this->ip_port = *ip_port_ptr;
        this->payload_len = payload_len;
        if(payload != NULL)
            memcpy(this->payload, payload, payload_len);
    }
    transfer_packet() : request_type(-1), payload_len(0)
    {
        for(int i = 0; i < 50; i++)
        {
            username[i] = filename[i] = 0;
        }
        for(int i = 0; i < TRANSFER_PACK_PAYLOADLEN; i++)
            payload[i] = 0;
        //empty packet, whose filed will be filled from server's reply
    }

    int32_t get_request_type()
    {
        return ntohl(request_type);
    }

    void set_request_type(int32_t request_type)
    {
        this->request_type = htonl(request_type);
    }

    int32_t get_payload_len()
    {
        return ntohl(payload_len);
    }

    void set_payload_len(int32_t payload_len)
    {
        this->payload_len = htonl(payload_len);
    }
    
    void set_filename(string &filename)
    {
        //strncpy(this->filename, filename.c_str(), filename.size());
        strncpy(this->filename, filename.c_str(), filename.size() + 1);
    }

    void set_username(string &username)
    {
        //strncpy(this->username, username.c_str(), username.size());
        strncpy(this->username, username.c_str(), username.size() + 1);
    }
};

struct connection
{
    int socket_fd;
    sockaddr_in ip_port;
    string filename;
    string username;

    FILE *fp;

    transfer_packet packet;
    char* packet_ptr;
    char* current_ptr;
    int remain_packet_len;

    connection() : socket_fd(-1), packet_ptr((char *)&packet), fp(NULL), current_ptr((char *)&packet), remain_packet_len(sizeof(packet))
    {
    }

    connection(const connection &conn)
    {
        this->socket_fd = conn.socket_fd;
        this->ip_port = conn.ip_port;
        this->filename = conn.filename;
        this->username = conn.username;
        this->fp = conn.fp;
        this->packet = conn.packet;

        this->packet_ptr = (char *)&this->packet;
        this->current_ptr = ((char *)&this->packet) + (conn.current_ptr - conn.packet_ptr);
        this->remain_packet_len = conn.remain_packet_len;
    }

    connection& operator=(const connection &conn)
    {
        this->socket_fd = conn.socket_fd;
        this->ip_port = conn.ip_port;
        this->filename = conn.filename;
        this->username = conn.username;
        this->fp = conn.fp;
        this->packet = conn.packet;

        this->packet_ptr = (char *)&this->packet;
        this->current_ptr = ((char *)&this->packet) + (conn.current_ptr - conn.packet_ptr);
        this->remain_packet_len = conn.remain_packet_len;
    }

    bool can_parse_packet()
    {
        return remain_packet_len == 0;// || remain_packet_len == sizeof(packet);
        // right of || is case that reset_buffer() have been called
    }

    bool is_write_completely_packet()
    {
        return remain_packet_len == 0;// || remain_packet_len == sizeof(packet);
        // right of || is case that reset_buffer() have been called
    }

    void read_from_socket()
    {
        int main_socket_read_length = read(socket_fd, current_ptr, remain_packet_len);
        if(main_socket_read_length < 0)
        {
            if(errno != EWOULDBLOCK)
            {
                cout << "read from socket error\n";
                exit(1);
            }
            else
            {
                cout << "read would block\n";
            }
        }
        else if(main_socket_read_length == 0)
        {
            cout << "server crashed\n";
            exit(0);
        }
        else
        {
            cout << "main_socket_read_length: " << main_socket_read_length << "\n";
            current_ptr += main_socket_read_length;
            remain_packet_len -= main_socket_read_length;
        }
    }

    void write_to_socket()
    {
        int main_socket_write_length = write(socket_fd, current_ptr, remain_packet_len);
        //EWOULDBLOCK ???????????
        if(main_socket_write_length < 0)
        {
            if(errno != EWOULDBLOCK)
            {
                cout << "write to socket error\n";
                exit(1);
            }
        }
        else
        {
            cout << "main_socket_write_length: " << main_socket_write_length << "\n";
            current_ptr += main_socket_write_length;
            remain_packet_len -= main_socket_write_length;
            cout << "remain_packet_len: " << remain_packet_len << "\n";
        }
    }

    void reset_buffer()
    {
        current_ptr = packet_ptr;
        remain_packet_len = sizeof(packet);
        string filename(packet.filename);
        set_filename(filename);
        string username(packet.username);
        set_username(username);
        set_sockaddr(packet.ip_port);
    }

    int32_t get_request_type()
    {
        /*
        if(!can_parse_packet())
        {
            cout << "packet are not received completely!\n";
            return -1;
        }
        else
        {
            return packet.get_request_type();
        }
        */
        return packet.get_request_type();
    }

    void set_request_type(int32_t request_type)
    {
        packet.set_request_type(request_type);
    }

    int32_t get_payload_len()
    {
        if(!can_parse_packet())
        {
            cout << "packet are not received completely!\n";
            return -1;
        }
        else
        {
            return packet.get_payload_len();
        }
    }

    void set_payload_len(int32_t payload_len)
    {
        packet.set_payload_len(payload_len);
    }

    sockaddr_in& get_sockaddr()
    {
        return ip_port;
    }

    void set_sockaddr(sockaddr_in& ip_port)
    {
        this->ip_port = ip_port;
    }

    void set_filename(string &filename)
    {
        this->filename = filename;
        packet.set_filename(filename);
    }

    string& get_filename()
    {
        return filename;
    }

    void set_username(string username)
    {
        this->username = username;
        packet.set_username(username);
    }

    void openfile_to_write()
    {
        fp = Fopen(filename.c_str(), "wb");
    }

    void openfile_to_read()
    {
        fp = Fopen(filename.c_str(), "rb");
    }

    void connect_by_sockaddr_in()
    {
        socket_fd = use_sockaddr_in_to_connect(ip_port);
    }

    void wrtie_payload_to_file()
    {
        fwrite(&packet.payload, get_payload_len(), sizeof(char), fp);
    }

    void read_payload_from_file()
    {
        int32_t payload_len =
            fread(&packet.payload, sizeof(packet.payload), sizeof(char), fp);
        set_payload_len(payload_len);
    }

    void close_file()
    {
        fclose(fp);
        fp = NULL;
    }

    void close_socket()
    {
        if(socket_fd == -1)
            return;
        else
            close(socket_fd);
    }

    bool file_is_close()
    {
        return fp == NULL;
    }
};

struct user
{
    string name;
    vector<string> filelist;
};

struct connection_server: public connection
{
    int listen_socket_fd;
    int read_write_flag;
    vector<string> filelist;

    connection_server() : listen_socket_fd(-1), read_write_flag(-1)
    {

    }

    void listen(const char* ip_str)
    {
        listen_socket_fd = new_tcp_listening_nonblocking_socket(ip_str);
    }

    int nonblocking_accept()
    {
        socket_fd = accept(listen_socket_fd, NULL, NULL);
        if(socket_fd < 0)
        {
            
            if(errno != EWOULDBLOCK)
            {
                cout << "errno: " << errno << "\n";
                cout << "accepting error\n";
                exit(1);
            }
            else
            {
                return -1;
            }
        }
        else
        {
            set_to_nonblocking(socket_fd);
            return socket_fd;
        }
    }
};



#endif