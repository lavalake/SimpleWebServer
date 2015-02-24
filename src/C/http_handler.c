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
/* Includes related to socket programming */
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "http_handler.h"
#define URLLEN 200 //max URL length
#define RSP_HEADER_LEN 65
/*
static char default_page[] = "../reference/www/index.html";
#define PATH "../reference/www/"
static char error_bad[] = {"HTTP/1.0 400 Bad Request\r\n"
    "Content-type: text/html\r\n\r\n"
    "<html><body>Error</body></html>\r\n\r\n"};
    */
static char server_info[] = "Server: Simple/1.0\r\n";
static char ok_rsp[] = "HTTP/1.0 200 OK\r\n";
//static char content_len[] = "Content-length: ";
static char content_type[] = "Content-type: ";


void parseRequest(int fd,char *request){

    HTTPRSP rsp;
    rsp.file_name = (char*)malloc(URLLEN);
    sendRsp(fd,rsp);
}
void sendRsp(int fd,HTTPRSP rsp){
    char header[RSP_HEADER_LEN];
    int total=0;
    int total_sent=0,bytes_sent=0;
    strcpy(header,ok_rsp);
    total = strlen(header);
    strcpy(header+total,server_info);
    total = strlen(header);
    strcpy(header+total,content_type);
    total = strlen(header);
    strcpy(header+total,"text/html\r\n\0");
    total = strlen(header);
    printf("rsp header %s\n",header);
    while (total_sent < total) {
        bytes_sent = send(fd, header+ total_sent,total- total_sent, 0);
            if (bytes_sent <= 0) {
                break;
            } else {
                total_sent += bytes_sent;
            }
    }

}
