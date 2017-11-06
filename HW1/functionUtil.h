#ifndef FUNCTION_UTIL
#define FUNCTION_UTIL

#include <vector>
#include <string>
#include "defineStruct.h"

using std::string;
using std::vector;
using std::max;
using std::stringstream;

void generate_ip_port_pair_c_str(const client_info &client, string &ip_port_pair_string);
string find_user_name_by_sockfd(vector<client_info> &clients, int client_socket_fd);
void clear_offline_user(vector<client_info> &clients, int client_socket_fd);
int find_user_name_by_string_name(vector<client_info> &clients, const string &name);
int max_fd(vector<client_info> &clients);
int send_welcome_and_add_client(int server_socket_fd, vector<client_info> &clients);
void brocast_to_others(int new_client_sockfd, vector<client_info> &clients);
void parse_command_and_send_message(vector<client_info> &clients, char *command_from_client, int client_socket_fd);

#endif