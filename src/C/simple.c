/* simple.c
* Author: jian wang xunrong li
* Date: 24 Feb, 2015
* This is a simple web server.The main process will 
* create a thread pool of 10 and pending on a working queue
* whenver the main process receive a TCP request, it will put
* the socke file descriptor into the working queue and wake up
* one working thread.
* We create thread pool to save the effor to create thread for
* every tcp connection. 
*/
/* Standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/* Includes related to socket programming */
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>

/* Local includes from ./inc */
#include <log.h>
#include <helper.h>
#include <config.h>
#include "http_handler.h"
#define ARGS_NUM 2
#define MAX_LINE 128
#define MAX_QUEUE_SIZE 50 //the queue size for the tcp connection
#define MAX_THREAD_NUM  10 //number of working thread
char *path;
char *root;
void handle_conn(void*);
//queue for the incoming tcp connection
//main process will put connection into queue
//working thread will take socket fd from queue
typedef struct queue{
    int socket_fd[MAX_QUEUE_SIZE];
    int length;
    int front;
    int rear;
    pthread_mutex_t qmutex;
    pthread_cond_t cond_null;
    pthread_cond_t cond_full;

} JOBQUEUE;
JOBQUEUE jq;
pthread_t tid[MAX_THREAD_NUM];
/*init()
 * initialze the mutex and conditional veriable and 
 * create the working thread pool
 */
void init(){
    int i=0;
    jq.length = 0;
    jq.front=jq.rear=0;
    pthread_mutex_init(&jq.qmutex,NULL);
    pthread_cond_init(&jq.cond_null,NULL);
    pthread_cond_init(&jq.cond_full,NULL);
    for(i=0;i<MAX_THREAD_NUM;i++){
        if(pthread_create(&tid[i],NULL,(void*)handle_conn,&tid[i]) < 0)
            fprintf(stdout,"pthread create error \n");
    }

}
void enQueue(int fd){
    jq.socket_fd[jq.rear] = fd;
    jq.rear = (jq.rear+1)%MAX_QUEUE_SIZE;
    jq.length++;
}
int deQueue(){
    int fd = jq.socket_fd[jq.front];
    jq.front = (jq.front+1)%MAX_QUEUE_SIZE;
    jq.length--;
    return fd;
}
/*handle_conn
 * the entry for working thread.
 * It will use select to wait for incoming data. 5 seconds
 * time out is set for select. The purpose of this timeout is
 * to defence the DOS attack.
 */
void handle_conn(void *input){
    int bytes_received=0;
    char buffer[MAX_LINE];
    int client_sock=0;
    fd_set read_set;
    struct timeval t;
    int result;
    printf("thread %d run\n", *(int*)input);
    while(1){
        pthread_mutex_lock(&jq.qmutex);
        printf("thread %d,q %d\n",*(int*)input,jq.length);
        //if the queue is empty, we need to wait for main process
        //put new connection into the queue
        while(jq.length <=0 ){
            printf("emty q, waiting %d\n", *(int*)input);
            pthread_cond_wait(&jq.cond_null,&jq.qmutex);
        }
        client_sock = deQueue();
        pthread_cond_signal(&jq.cond_full);
        printf("working thread %d fd %d\n", *(int*)input, client_sock);
        pthread_mutex_unlock(&jq.qmutex);
        FD_ZERO(&read_set);
        FD_SET(client_sock,&read_set);
        t.tv_sec = 5;
        t.tv_usec = 0;
        result = select(client_sock+1, &read_set, NULL, NULL, &t);
        if(result > 0){
            if(FD_ISSET(client_sock, &read_set)){
                bytes_received = recv(client_sock, buffer, MAX_LINE - 1, 0);
            }
        }else{
            
            printf("thead %d select timeout Closing connection %d",
                *(int*)input, client_sock );
            /* Our work here is done. Close the connection to the client */
            close(client_sock);
            continue;
        }
        
        buffer[bytes_received] = '\0';
        //printf("parse http request\n");
        parseRequest(client_sock,buffer);
        printf("thead %d Closing connection %d",
            *(int*)input, client_sock );
        /* Our work here is done. Close the connection to the client */
        close(client_sock);
    }
}
int main(int argc, char **argv)
{
    int port;
    int serv_sock, client_sock;
    int optval = 1;
    int bytes_received, bytes_sent, total_sent;

    socklen_t len;
    struct sockaddr_in addr, client_addr;

    char *client_addr_string;

    /*
     * ignore SIGPIPE, will be handled
     * by return value
     */
    signal(SIGPIPE, SIG_IGN);

    if (argc != (ARGS_NUM + 1)) {
        error_log("%s","Incorrect arguments provided\n"
                  "usage: ./server <port> <root folder>");

        exit(EXIT_FAILURE);
    }

    init();

    client_addr_string = (char *) malloc(INET_ADDRSTRLEN);
    if (NULL == client_addr_string) {
        error_log("Unable to allocate memory for client_addr due to malloc() "
                  "error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Parse the port */
    port = atoi(argv[1]);
    if ((port > MAX_PORT) || (port < MIN_PORT)) {
        error_log("%s", "Port must be in range 1024 to 65535");
        exit(EXIT_FAILURE);
    }
    path = argv[2];
    root = (char*)malloc(strlen(path)+1);
    strcpy(root,path);
    strcpy(root+strlen(root),"/");

    /* Create a socket for listening */
    if ((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error_log("socket() error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(port);

    /*
     * bind() the socket to the ip address and the port. This actually
     * created the mapping between the socket and the IP:Port pair
     */
    if (bind(serv_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        error_log("bind() error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval, sizeof(int)) < 0) {
        error_log("setsockopt() error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /*
     * listen() for incoming connections.
     * The server accepts SYN packets after this call succeeds. If a client
     * calls connect() before this, the OS kernel will response with a RST and
     * terminate the connection. The BACKLOG is the total amount of connections
     * that can be in ESTABLISHED state (SYN-SYN/ACK-ACK complete at the server
     * before accept() is called.
     */
    if (listen(serv_sock, BACKLOG) < 0) {
        error_log("listen() error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    debug_log("Now listening on port %d", port);
    printf("Now listening on port %d\n",port);
    while(1) {
        total_sent = bytes_received = bytes_sent = 0;
        /* Accept the client connection  */
        len = sizeof(client_addr);

        /* Get a socket to actually communicate withe client
         * The client_sock returned by accept() is used for further
         * communication with the client while the serv_sock is still used for
         * new connections. accept() blocks if no connections are present
         */
        client_sock = accept(serv_sock,
                             (struct sockaddr *) &client_addr, &len);
        if (client_sock < 0) {
            error_log("Unable to add client due to accept() "
                      "error: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("Accepted connection from %s:%d",
                  client_addr_string, ntohs(client_addr.sin_port));

        inet_ntop(AF_INET, &(client_addr.sin_addr),
                  client_addr_string, INET_ADDRSTRLEN);
        debug_log("Accepted connection from %s:%d",
                  client_addr_string, ntohs(client_addr.sin_port));
        pthread_mutex_lock(&jq.qmutex);
        printf("put socket %d to q, %d\n",client_sock,jq.length);
        while(jq.length == MAX_QUEUE_SIZE){
            pthread_cond_wait(&jq.cond_full,&jq.qmutex);
        }
        enQueue(client_sock);
        pthread_cond_signal(&jq.cond_null);
        pthread_mutex_unlock(&jq.qmutex);
    }
    return 0;
}
