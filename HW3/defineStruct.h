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

    void set_sockaddr(sockaddr_in &ip_port)
    {
        this->ip_port = ip_port;
    }

    sockaddr_in get_sockaddr()
    {
        return this->ip_port;
    }
};

struct connection
{
    int socket_fd;
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
        this->filename = conn.filename;
        this->username = conn.username;
        this->fp = conn.fp;
        this->packet = conn.packet;

        this->packet_ptr = (char *)&this->packet;
        this->current_ptr = ((char *)&this->packet) + (conn.current_ptr - conn.packet_ptr);
        this->remain_packet_len = conn.remain_packet_len;
    }

    int get_socket_fd()
    {
        return socket_fd;
    }

    string get_username()
    {
        return username;
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

    int read_from_socket()
    {
        int main_socket_read_length = read(socket_fd, current_ptr, remain_packet_len);
        if(main_socket_read_length < 0)
        {
            if(errno != EWOULDBLOCK)
            {
                cout << "read from socket error\n";
                exit(1);
            }
        }
        else
        {
            cout << "main_socket_read_length: " << main_socket_read_length << "\n";
            current_ptr += main_socket_read_length;
            remain_packet_len -= main_socket_read_length;
        }
        return main_socket_read_length;
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

    void reset_buffer_and_fill_member_from_packet()
    {
        current_ptr = packet_ptr;
        remain_packet_len = sizeof(packet);
        string filename(packet.filename);
        set_filename(filename);
        string username(packet.username);
        set_username(username);
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
        /*
        if(!can_parse_packet())
        {
            cout << "packet are not received completely!\n";
            return -1;
        }
        else
        {
            return packet.get_payload_len();
        }
        */
        return packet.get_payload_len();
    }

    void set_payload_len(int32_t payload_len)
    {
        packet.set_payload_len(payload_len);
    }

    sockaddr_in get_sockaddr()
    {
        return packet.get_sockaddr();
    }

    void set_sockaddr(sockaddr_in ip_port)
    {
        packet.set_sockaddr(ip_port);
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
        socket_fd = use_sockaddr_in_to_connect(packet.ip_port);
    }

    void wrtie_payload_to_file()
    {
        fwrite(&packet.payload, sizeof(char), get_payload_len(), fp);
    }

    void read_payload_from_file()
    {
        int32_t payload_len =
            fread(&packet.payload, sizeof(char), sizeof(packet.payload), fp);
        set_payload_len(payload_len);
    }

    void close_file()
    {
        if(fp != NULL)
        {   
            fclose(fp);
            fp = NULL;
        }
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
    int listen_socket_fd = -1;
    int read_write_flag = 0;
    vector<string> filelist;

    int32_t listen_socket_state_machine = 0;

    connection_server() : listen_socket_fd(-1), read_write_flag(-1)
    {

    }

    void listen(const char* port_str)
    {
        listen_socket_fd = new_tcp_listening_nonblocking_socket(port_str);
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

    void append_file_to_list(string &filename)
    {
        filelist.push_back(filename);
    }

    size_t get_filelist_size()
    {
        return filelist.size();
    }

    string pop_filename()
    {
        string filename = filelist.back();
        filelist.pop_back();
        return filename;
    }

    void initialize_filelist(vector<user> &users_filelist)
    {
        for(auto &user : users_filelist)
        {
            if(this->username == user.name)
            {
                this->filelist = user.filelist;
                return;
            }
        }
    }

    int32_t get_download_socket_state()
    {
        return listen_socket_state_machine;
    }

    void set_download_socket_state(int32_t state)
    {
        listen_socket_state_machine = state;
    }

    void set_sockaddr_by_getsockname()
    {
        socklen_t socklen = sizeof(sockaddr_in);
        getsockname(this->listen_socket_fd, (sockaddr *) &(packet.ip_port), &socklen);
    }

    void close_listening_socket()
    {
        if(listen_socket_fd == -1)
            return;
        else
            close(listen_socket_fd);
    }

    void openfile_to_write_server()
    {
        fp = Fopen((filename + "-" + username).c_str(), "wb");
    }

    void openfile_to_read_server()
    {
        fp = Fopen((filename + "-" + username).c_str(), "rb");
    }
    // 用後綴來防止不同 users 上傳相同檔名的情況
};



#endif