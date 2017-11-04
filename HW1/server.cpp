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
#include <vector>
#include <algorithm>

//#include <Server.h>

#define MAXLINE 2000
#define SERVERCOUNT 1
#define SERVER_PROMPT "[Server]"
#define BROCAST_STR "[Server] Someone is coming!\n"
#define NEW_COME_STR "[Server] Hello, anonymous! From:"

using std::string;
using std::vector;
using std::max;
//using std::to_string;

struct client_info
{
    sockaddr_in ip_port;
    string name = "anonymous";
    int socket_fd;
};

const char* generate_ip_port_pair_str(const client_info& client)
{
    char buff[20];
    inet_ntop(AF_INET, &client.ip_port.sin_addr, buff, sizeof(buff));

    return ((string(buff) + "/") + std::to_string(htons(client.ip_port.sin_port))).c_str();
}

int max_fd(vector<client_info> &clients)
{
    int temp_fd = 0;
    printf("temp_fd: %d\n", temp_fd);
    for(auto client : clients)
    {
        if(client.socket_fd > temp_fd)
            temp_fd = client.socket_fd;
    }
    return temp_fd;
}

int send_welcome_and_add_client(int server_socket_fd, vector<client_info> &clients)
{
    client_info new_client;
    bzero(&new_client.ip_port, sizeof(sockaddr_in));
    socklen_t len;
    new_client.socket_fd =
        accept(server_socket_fd, (sockaddr*)&new_client.ip_port, &len);
    
    string new_come_message = 
            string(NEW_COME_STR) + generate_ip_port_pair_str(new_client) + "\n";

    printf("RRRRRRRRRRR: %s\n", generate_ip_port_pair_str(new_client));

    clients.push_back(new_client);
    write(new_client.socket_fd, new_come_message.c_str(), strlen(new_come_message.c_str()));

    return new_client.socket_fd;
}

void brocast_to_others(int new_client_sockfd, vector<client_info> &clients)
{
    for(auto client : clients)
    {
        if(client.socket_fd == new_client_sockfd)
            continue;
        else
        {
            write(client.socket_fd, BROCAST_STR, strlen(BROCAST_STR));
        }
    }
}

void parse_command_and_send_message(vector<client_info> &clients, char *command_from_client, int client_socket_fd)
{
    if(string("who\n") == command_from_client)
    {
        //write(client_socket_fd, "WHO!!!\n", strlen("WHO!!!\n"));
        string list;
        for(auto client : clients)
        {
            list += SERVER_PROMPT;
            list += " ";
            list += client.name;
            list += " ";
            //list += generate_ip_port_pair_str(client);
            if(client.socket_fd == client_socket_fd)
                list += " -> me";
            list += "\n";
        }
        write(client_socket_fd, list.c_str(), list.size());
    }
    else if(string(command_from_client).substr(0, 5) == "yell ")
    {
        write(client_socket_fd, "YELL!!\n", strlen("YELL!!\n"));
    }
    else if(string(command_from_client).substr(0, 5) == "tell ")
    {
        write(client_socket_fd, "TELL!!\n", strlen("TELL!!\n"));
    }
    else if(string(command_from_client).substr(0, 5) == "name ")
    {
        write(client_socket_fd, "NAME!!\n", strlen("NAME!!\n"));
    }
    else
    {
        write(client_socket_fd, "ERROR!!!!!\n", strlen("ERROR!!!!!\n"));
    }
}


int main(int argc, char **argv)
{
    if(argc < 2)
    {
        printf("./server <SERVER_PORT>\n");
        return 1;
    }
    //create server socket
    int server_socket_fd;
    if( (server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("open socket failed.\n");
        exit(1);
    }

    //bind socket
    sockaddr_in server_ip_port;
    bzero(&server_ip_port, sizeof(server_ip_port));
    server_ip_port.sin_family = AF_INET;
    server_ip_port.sin_port = htons(atoi(argv[1]));
    server_ip_port.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(server_socket_fd, (sockaddr *)&server_ip_port, sizeof(server_ip_port)) == -1)
    {
        printf("bind socket error\n");
        exit(1);
    }

    //listen socket
    listen(server_socket_fd, 15);
    
    vector<client_info> clients;
    //store all clients' information
    
    while(true)
    {
        //----------------select routine---------------
        fd_set reading_fds;
        //fd_set writing_fds;

        struct timeval tv;
        
        tv.tv_sec = 1;
        tv.tv_usec = 500000;

        //reading fds
        FD_ZERO(&reading_fds);

        //add all clients' socketfd and server's socketfd to reading_fds
        printf("set fd\n");
        FD_SET(server_socket_fd, &reading_fds);
        for(int i = 0; i < clients.size(); i++)
        {
            FD_SET(clients[i].socket_fd, &reading_fds);
        }
        
        printf("clients' size: %u\n", clients.size());
        int maxfdp = max(max_fd(clients), server_socket_fd);
        printf("set fd complete\n");

        printf("select:\n");
        switch(select(maxfdp + 1, &reading_fds, NULL, NULL, &tv))
        {
            case -1:
                printf("select error\n");
                exit(1);
                break;
            case 0:
                printf("no client\n");
                break;
            default:
                printf("default\n");
                if(FD_ISSET(server_socket_fd, &reading_fds))// new connection from client
                {
                    printf("new client!!!\n");
                    int new_client_sockfd = send_welcome_and_add_client(server_socket_fd, clients);
                    printf("add new client complete\n");
                    brocast_to_others(new_client_sockfd, clients);
                }
                else
                {

                    char command_from_client[MAXLINE];
                    for(int i = 0; i < clients.size(); i++)
                    {
                        if(FD_ISSET(clients[i].socket_fd, &reading_fds)) // some client sends command to server
                        {
                            int n = read(clients[i].socket_fd, command_from_client, MAXLINE);
                            command_from_client[n] = 0;
                            
                            printf("client %d's command: %s", clients[i].socket_fd, command_from_client);

                            parse_command_and_send_message(clients, command_from_client, clients[i].socket_fd);

                            write(clients[i].socket_fd, SERVER_PROMPT, strlen(SERVER_PROMPT));
                        }
                    }
                } 
                break;          
        }
        printf("select complete\n");
    }
}