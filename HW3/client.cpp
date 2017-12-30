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
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>

#define MAXLINE 1460
//int initialize_socket(char *ip_str, char *port_str,
/* value-result arg*/ //int *socket_fd, struct *sockaddr_in sock_struct);

using std::string;
using std::cin;
using std::vector;
using std::stringstream;
using std::max;

struct transfer_record
{
    int64_t request; 
    /*-1: constructor state
     * 0: greeting
     * 1: client put command, file transfer request
     * 2: server reply for put
     * 3: get file from server request when login
     * 4: server reply for get
     * 5: server actively says there is a file for you
     * 6: client reply for 5
     * 7: payload is file content
    */
    char payload[MAXLINE];
    /*-1: no content
     * 0: greeting string
     * 1: filename
     * 2: no content
     * 3: filename
     * 4: no content
     * 3: file content
    */
    transfer_record()
    {
        request = -1;
        payload[0] = '\0';
    }
};

class nonblockingSocket
{
private:
    char buffer_to_dest[MAXLINE];
    char *to_dest_remain_ptr; // point to address that can insert data from file
    char *to_dest_ready_ptr; // point to data that haven't been sent to server

    char buffer_from_dest[MAXLINE];
    char *from_dest_remain_ptr; // point to address that can insert date from server
    char *from_dest_ready_ptr; // point to data that haven't been sent to local file

    FILE *fp;
    string filename;
    struct transfer_record record;
    int64_t start_session_type;

public:
    int socket_fd;
    nonblockingSocket(char *IP_str, char *port_str, int64_t session_type);
    void set_to_nonblocking();
    void open_upload_file(string filename);
    void send_transfer_request()
    {

    }
    void send_data(int length_of_send_data, char *src);
    void receive_data(int &length_of_receive_data, char *dest);


    //need close function destructor?

private:
    int new_tcp_connection(char *IP_str, char *port_str);
};

nonblockingSocket::nonblockingSocket(char *IP_str, char *port_str, int64_t session_type)
{
    fp = NULL;
    socket_fd = new_tcp_connection(IP_str, port_str);
    to_dest_ready_ptr = to_dest_remain_ptr = buffer_to_dest;
    from_dest_ready_ptr = from_dest_remain_ptr = buffer_from_dest;
    // initialize variable


    session_type ? set_to_nonblocking() : false;
    // 改成看這個 type 決定這個 socket 的起始狀態是哪個
}

void nonblockingSocket::set_to_nonblocking()
{
    int fcntl_val = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, fcntl_val | O_NONBLOCK);
    // set socket to nonblocking
}

int nonblockingSocket::new_tcp_connection(char *IP_str, char *port_str)
{
    int socket_fd;
    if( (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("open socket failed.\n");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port =  htons(atoi(port_str));

    if(!inet_pton(AF_INET, IP_str, &server_addr.sin_addr))
    {
        printf("fuckyou invalid IP address\n");
        exit(1);
    }

    if (connect(socket_fd, (sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    {
        printf("connect error\n");
        exit(1);
    }
    return socket_fd;
}

void nonblockingSocket::open_upload_file(string filename)
{
    if( (fp = fopen(filename.c_str(), "r")) < 0)
    {
        std::cout << "openfile error\n";
        exit(1);
    }
}

#define BLOCKING false

int main(int argc, char ** argv)
{

    if(argc < 4)
    {
        printf("./client <ip> <port> <username>");
        return 1;
    }

    nonblockingSocket main_socket(argv[1], argv[2], BLOCKING);

    /*
    int main_socket_fd = new_tcp_connection(argv[1], argv[2]);
    int fcntl_val = fcntl(main_socket_fd, F_GETFL, 0);
    fcntl(main_socket_fd, F_SETFL, fcntl_val | O_NONBLOCK);
    */

    int n;
    char recvline[MAXLINE];
    recvline[0] = 0;
    char commandline[MAXLINE];
    commandline[0] = 0;
    vector<nonblockingSocket> download_file_connections;
    vector<nonblockingSocket> upload_file_connections;

    while (true) 
    {
        fd_set reading_fds;
        struct timeval tv;
    
        tv.tv_sec = 0;
        tv.tv_usec = 500;
        FD_ZERO(&reading_fds);
        FD_SET(STDIN_FILENO, &reading_fds);
        FD_SET(main_socket.socket_fd, &reading_fds);
        int maxfdp = max(STDIN_FILENO, main_socket.socket_fd) + 1;
        // select only for stdin

        int new_command_flag = 0, server_reply_flag = 0;
        switch (select(maxfdp, &reading_fds, NULL, NULL, &tv))
        {
            case -1:
                exit(-1);
                break;
            case 0:
                break;
            default:
                if(FD_ISSET(STDIN_FILENO, &reading_fds))
                {
                    new_command_flag = 1;
                    if(fgets(commandline, MAXLINE, stdin) == NULL)
                        exit(0);
                    // read commandline, but the content will be parsed later
                }
                if(FD_ISSET((main_socket.socket_fd, &reading_fds))
                {
                    server_reply_flag = 1;
                    //read to main_socket's record
                }
                if()
                {
                    
                }
            break;
        }

        if(new_command_flag)
        {   
            stringstream SS;
            SS << commandline;
            string command;
            SS >> command;
            if(string("/put") == command)
            {
                string filename;
                SS >> filename;
            }
            else if(string("/sleep") == command)
            {
                int sleep_time;
                SS >> sleep_time;
                int counter = 0;
                while(sleep_time--)
                {
                    counter++;
                    sleep(1);
                    std::cout << "Sleep " << counter << "\n";
                }
                std::cout << "Client wakes up\n";
            }
            else if(string("/exit") == command)
            {
                exit(0);
            }
        }
    }
}
