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
#define RSP_HEADER_LEN 65
#define BUF_SIZE 1024
static char default_page[] = "index.html";
#define HTTPGET 0
#define HTTPHEAD 1
#define HTTPOTHER 2
/*
static char error_bad[] = {"HTTP/1.0 400 Bad Request\r\n"
    "Content-type: text/html\r\n\r\n"
    "<html><body>Error</body></html>\r\n\r\n"};
    */
static char error_unsupport[] = "HTTP/1.0 501 Method Unimplemented";
static char server_info[] = "Server: Simple/1.0\r\n";
static char ok_rsp[] = "HTTP/1.0 200 OK\r\n";
//static char content_len[] = "Content-length: ";
static char content_type[] = "Content-type: ";


void parseRequest(int fd,char *request){

    HTTPRSP rsp;
    int path_len=0;
    rsp.file_name = (char*)malloc(URLLEN);
    strcpy(rsp.file_name,path);
    path_len = strlen(path);
    strcpy(rsp.file_name+path_len,default_page);
    sendRsp(fd,rsp);
}
void sendRsp(int fd,HTTPRSP rsp){
    char header[RSP_HEADER_LEN];
    char file_buf[BUF_SIZE];
    char *file_type;
    int total_len=0,read_length=0;
    int total=0;
    int total_sent=0,bytes_sent=0;
    int read_fd;
    struct stat stat_buf;
    if(rsp.http_method == HTTPOTHER){
        send_error_rsp(fd);
    }

    strcpy(header,ok_rsp);
    total = strlen(header);
    strcpy(header+total,server_info);
    total = strlen(header);
    strcpy(header+total,content_type);
    total = strlen(header);
    file_type = get_mime(rsp.file_name);
    printf("file type %s\n",file_type);
    strcpy(header+total,file_type);
    total = strlen(header);
    strcpy(header+total,"\r\n");
    total += 2;
    printf("rsp header %s\n",header);
    while (total_sent < total) {
        bytes_sent = send(fd, header+ total_sent,total- total_sent, 0);
            if (bytes_sent <= 0) {
                break;
            } else {
                total_sent += bytes_sent;
            }
    }
    if(rsp.http_method == HTTPHEAD){
        return;
    }    
    printf("file name %s\n",rsp.file_name);
    read_fd = open(rsp.file_name,O_RDONLY);
    fstat(read_fd,&stat_buf);
    while(total_len < stat_buf.st_size){
       int total_send = 0;
       read_length = read(read_fd,file_buf,BUF_SIZE);
       total_len += read_length;
       while(total_send < read_length){
           int send_len = 0;
           send_len = send(fd,file_buf+total_send,read_length-total_send,0);
           if(send_len < 0){
               break;
           }else{
               total_send += send_len;
           }
       }
    }

}
void send_error_rsp(int fd){
    int len = 0;
    int total_send = 0;
    len = strlen(error_unsupport);
   while(total_send < len){
       int send_len = 0;
       send_len = send(fd,error_unsupport+total_send,len-total_send,0);
       if(send_len < 0){
           break;
       }else{
           total_send += send_len;
       }
   }

}
void get_filetype(char *name, char *type) {
        if (strstr(name, ".html"))
            strcpy(type, "text/html");
        else if (strstr(name, ".png"))
            strcpy(type, "image/png");
        else if (strstr(name, ".jpg"))
            strcpy(type, "image/jpeg");
        else if (strstr(name, ".gif"))
            strcpy(type, "image/gif");
        else
            strcpy(type, "text/plain");
};
