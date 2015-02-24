typedef struct response{
        int http_method;
            char *file_name;
} HTTPRSP;
void parseRequest(int fd,char *request);
void sendRsp(int fd,HTTPRSP rsp);
