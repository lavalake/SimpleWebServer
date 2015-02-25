extern char *path;
typedef struct response{
        int http_method;
            char *file_name;
} HTTPRSP;
void parseRequest(int fd,char *request);
void handleStatic(int fd,HTTPRSP rsp);
void handleDyn(int fd,HTTPRSP rsp,char *args);
void sendRspHeader(int fd,HTTPRSP rsp);
void send_error_rsp(int fd,char *err);
