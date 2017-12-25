

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
