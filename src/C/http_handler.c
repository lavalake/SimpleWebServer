/*http_handler.c 
 * Author: jian wang xunrong li
 * Date: 24 Feb, 2015
 * This is a handler to parse the http request
 * and send http response
 */
/* Standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
/* Includes related to socket programming */
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "http_handler.h"
#include <helper.h>
#define URLLEN 200 //max URL length
#define RSP_HEADER_LEN 650
#define BUF_SIZE 4096
static char default_page[] = "/index.html";
#define HTTPGET 0
#define HTTPHEAD 1
#define HTTPOTHER 2
/*
static char error_bad[] = {"HTTP/1.0 400 Bad Request\r\n"
    "Content-type: text/html\r\n\r\n"
    "<html><body>Error</body></html>\r\n\r\n"};
    */
static char error_not_found[] = "HTTP/1.0 4.4 Page Not Found";
static char error_unsupport[] = "HTTP/1.0 501 Method Unimplemented";
static char server_info[] = "Server: Simple/1.0\r\n";
static char ok_rsp[] = "HTTP/1.0 200 OK\r\n";
//static char content_len[] = "Content-length: ";
static char content_type[] = "Content-type: ";


void parseRequest(int fd, char *request){

    HTTPRSP rsp;
    int path_len=0;

	  char method[BUF_SIZE], uri[BUF_SIZE];
    char *buf = (char*) malloc(BUF_SIZE);
    if(buf == NULL){
        printf("malloc failed\n");
    }
    rsp.file_name = (char*) malloc(BUF_SIZE);

    //split the request content by "\r\n"
    buf = strtok(request, "\r\n");
    printf("%s\n", buf);
    sscanf(buf, "%s %s", method, uri);

    if (strcasecmp(method, "GET") == 0){
      rsp.http_method = 0;
    }
    else if (strcasecmp(method, "HEAD")){
      rsp.http_method = 1;
    }
    else{
      //501 error
      printf("%s\n", "501 Method Unimplemented");
    }
    
    //printf("%s\n", "printing out the method");
    printf("%d\n", rsp.http_method);
    //printf("%s\n", "this is going to be the filename");
    //printf("%s\n", uri);

    strcpy(rsp.file_name,path);
    path_len = strlen(path);
    strcpy(rsp.file_name+path_len, uri);
    
    handleStatic(fd, rsp);
}


void sendRspHeader(int fd,HTTPRSP rsp){
    char header[RSP_HEADER_LEN];
    int path_len;
    int total=0;
    int total_sent=0,bytes_sent=0;
    char *file_type;
    //struct stat stat_buf;
    strcpy(header,ok_rsp);
    total = strlen(header);
    strcpy(header+total,server_info);
    total = strlen(header);
    strcpy(header+total,content_type);
    total = strlen(header);
    //printf("file name %s\n",rsp.file_name);
    path_len = strlen(path);
    if(strcmp(rsp.file_name,root) == 0){
        int path_len = strlen(path);
        strcpy(rsp.file_name,path);
        strcpy(rsp.file_name+path_len,default_page);
        printf("chage to default file %s\n",rsp.file_name);
    }
    file_type = get_mime(rsp.file_name);
    
    //printf("file type %s\n",file_type);
    strcpy(header+total,file_type);
    total = strlen(header);
    strcpy(header+total,"\r\n");
    total += 2;
    /*
    if(stat(rsp.file_name,&stat_buf) == 0){
        char str[100];
        sprintf(str, " %d" , (int)stat_buf.st_size);
        strcpy(header+total,content_len);
        total = strlen(header);
        strcpy(header+total,str);
        total = strlen(header);
        strcpy(header+total,"\r\n");
        total += 2;
        printf("conten len %s\n",str);
    }
*/
    strcpy(header+total,"\r\n");
    total += 2;
    //printf("rsp header %s\n",header);
    while (total_sent < total) {
        bytes_sent = send(fd, header+ total_sent,total- total_sent, 0);
            if (bytes_sent <= 0) {
                break;
            } else {
                total_sent += bytes_sent;
            }
    }

    printf("send header finished\n");
}
void handleStatic(int fd,HTTPRSP rsp){
    char file_buf[BUF_SIZE];
    int total_len=0,read_length=0;
    int read_fd;
    struct stat stat_buf;
    if(rsp.http_method == HTTPOTHER){
        send_error_rsp(fd,error_unsupport);
        return;
    }
    sendRspHeader(fd,rsp);


    if(rsp.http_method == HTTPHEAD){
        printf("http head, just return");
        return;
    }    
    //printf("file name %s\n",rsp.file_name);
    
    if(stat(rsp.file_name,&stat_buf) != 0){
        printf("file not exist\n");
        send_error_rsp(fd,error_not_found);
        return;
    }
    printf("read and send file %s\n",rsp.file_name);
    read_fd = open(rsp.file_name,O_RDONLY);
    while(total_len < stat_buf.st_size){
       int total_send = 0;
       read_length = read(read_fd,file_buf,BUF_SIZE);
       //printf("read %d from file\n",read_length);
       total_len += read_length;
       while(total_send < read_length){
           int send_len = 0;
           send_len = send(fd,file_buf+total_send,read_length-total_send,0);
           if(send_len <= 0){
               //there is some error occur or client close the connection
               printf("send error %d\n",errno);
               break;
           }else{
               //printf("send %d \n",send_len);
               total_send += send_len;
           }
       }
    }

}
void handleDyn(int fd,HTTPRSP rsp, char *args){
    pid_t id=0;
    int status;
    if(rsp.http_method == HTTPOTHER){
        send_error_rsp(fd,error_unsupport);
        return;
    }
    sendRspHeader(fd,rsp);

    if(rsp.http_method == HTTPHEAD){
        return;
    }    

    printf("file name %s\n",rsp.file_name);
    id = fork();
    if(id == 0){
        setenv("QUERY_STRING", args, 1);
        dup2(fd, STDOUT_FILENO);
        execve(rsp.file_name,NULL,NULL);
    }else{
        wait(&status);
    }
}
void send_error_rsp(int fd,char *err){
    int len = 0;
    int total_send = 0;
    len = strlen(err);
   while(total_send < len){
       int send_len = 0;
       send_len = send(fd,err+total_send,len-total_send,0);
       if(send_len < 0){
           break;
       }else{
           total_send += send_len;
       }
   }

}
