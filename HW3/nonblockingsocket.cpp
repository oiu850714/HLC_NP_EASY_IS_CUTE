class nonblockingSocket
{
    
    char buffer_to_dest[MAXLINE];
    char *to_dest_remain_ptr; // point to address that can insert data from file
    char *to_dest_ready_ptr; // point to data that haven't been sent to server

    char buffer_from_dest[MAXLINE];
    char *from_dest_remain_ptr; // point to address that can insert date from server
    char *from_dest_ready_ptr; // point to data that haven't been sent to local file

public:
    int socket_fd;
    nonblockingSocket(char *IP_str, char *port_str);
    void send_data(int length_of_send_data, char *src);
    void receive_data(int &length_of_receive_data, char *dest);

private:
    int new_tcp_connection(char *IP_str, char *port_str);
};

nonblockingSocket::nonblockingSocket(char *IP_str, char *port_str)
{
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