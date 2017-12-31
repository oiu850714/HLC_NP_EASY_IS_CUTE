#ifndef DEFINE_CONSTANT
#define DEFINE_CONSTANT


#define MAXLINE 1460
#define USERNAMELEN 50
#define FILENAMELEN 50

#define TRANSFER_PACK_PAYLOADLEN (MAXLINE-sizeof(int32_t)-50-50-sizeof(sockaddr_in)-sizeof(int32_t))

#define EMPTY_STR ""

#define SERV_RQ_GREETING 0
#define SERV_RQ_DOWNLOAD 1 // there is file to be download
#define SERV_RQ_DOWNLOADING 2 // payload is file content
#define SERV_RQ_DOWNFIN  3 // file downloading finished
#define SERV_RQ_UPLOAD 4
#define CLI_RQ_UPLOADING 5
#define CLI_RQ_UPFIN 6
#define CLI_RQ_USERNAME 7
#define CLI_RQ_UPLOADFILE 8
#define CLI_RQ_UPLOAD_CONNECTION 9


#define FTM_OPEN_LISTEN_SOCKET 0
#define FTM_WRITE_SOCKADDR_TO_CLI 1
#define FTM_LISTEN_CLI 2
#define FTM_ACCEPT_CLI 3

#define LISTEN_ARBITRARY_PORT "0"
#endif