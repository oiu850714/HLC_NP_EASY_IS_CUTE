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

#define MAXLINE 1460
//int initialize_socket(char *ip_str, char *port_str,
/* value-result arg*/ //int *socket_fd, struct *sockaddr_in sock_struct);

using std::string;
using std::cin;
using std::vector;
using std::stringstream;

struct transfer_record
{
    int64_t request; 
    /* 0: greeting
     * 1: client put command, file transfer request
     * 2: server reply for put
     * 3: server transfer file to [new connected client request| same user that puts file, but on different clients]
     * 4: client reply for downloading file
     * 5: payload is file content
    */
    char payload[MAXLINE];
    /* 0: greeting string
     * 1: filename
     * 2: no content
     * 3: filename
     * 4: no content
     * 3: file content
    */
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
    struct transfer_record record;

public:
    int socket_fd;
    nonblockingSocket(char *IP_str, char *port_str);
    void open_upload_file(string filename);
    void send_data(int length_of_send_data, char *src);
    void receive_data(int &length_of_receive_data, char *dest);


    //need close function destructor?

private:
    int new_tcp_connection(char *IP_str, char *port_str);
};

nonblockingSocket::nonblockingSocket(char *IP_str, char *port_str)
{
    fp = NULL;
    socket_fd = new_tcp_connection(IP_str, port_str);
    to_dest_ready_ptr = to_dest_remain_ptr = buffer_to_dest;
    from_dest_ready_ptr = from_dest_remain_ptr = buffer_from_dest;
    // initialize variable

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

int main(int argc, char ** argv)
{

    if(argc < 4)
    {
        printf("./client <ip> <port> <username>");
        return 1;
    }

    nonblockingSocket main_socket(argv[1], argv[2]);

    /*
    int main_socket_fd = new_tcp_connection(argv[1], argv[2]);
    int fcntl_val = fcntl(main_socket_fd, F_GETFL, 0);
    fcntl(main_socket_fd, F_SETFL, fcntl_val | O_NONBLOCK);
    */


    //================= set 

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
        int maxfdp = STDIN_FILENO + 1;
        // select only for stdin

        int new_command_flag = 0;
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

            }
        }
    }
}
