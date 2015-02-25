typedef struct response{
        int http_method; // 0 for get, 1 for Head, 2 for others
            char *file_name;
} HTTPRSP;
void parseRequest(int fd,char *request);
void sendRsp(int fd,HTTPRSP rsp);
