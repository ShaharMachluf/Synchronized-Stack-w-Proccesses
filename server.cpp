/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
// #include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include "stack.cpp"
#include "part5.cpp"

#define PORT 3490  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold
char *head, *bottom;
int *index_s;
pthread_mutex_t my_mutex;
struct flock lock;
int sockets[15];

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void *start_process(int *fd, int index){
    int sock = *fd;
    int flag = 1, i=0, bytes_get = 0;
    char command[1024];
    char data[1024];
    while(flag){
        memset(data, 0, 1024);
        memset(command, 0, 1024);
        bytes_get = 0;
        while(bytes_get == 0){
            bytes_get = recv(sock, data, 1024, 0);//recieve command from the client
        }
        if(data[0] == '\0'){
            continue;
        }
        i=0;
        while (data[i] != '\0' && data[i]!= ' ')//move the command part (PUSH/POP/TOP) to "command"
        {
            command[i] = data[i];
            i++;
        }
        // pthread_mutex_lock(&my_mutex);
        for(int k=0; k<15;k++){//lock all other sockets
            if(k!= index && sockets[i] != -1){
                fcntl(sockets[k], F_SETLK, &lock);
            }
        }
        if(strcmp(command, "PUSH")==0){
            int j=0;
            i++;
            while(data[i]!='\0'){//move the data part (without the push) to "data"
                data[j]=data[i];
                j++;
                i++;
            }
            data[j] = '\0';
            push(data, head+*index_s);
            (*index_s)+=strlen(data);
            if (send(sock, "sent", sizeof("sent"), 0) == -1)//confirm it worked
                perror("send failed\n");
            // pthread_mutex_unlock(&my_mutex);
            for(int k=0; k<15;k++){
                if(k!= index && sockets[i] != -1){
                    fcntl(sockets[k], F_UNLCK, &lock);
                }
            }
            continue;
        }
        else if(strcmp(command, "POP\n")==0){
            if(*index_s == 0){
                if (send(sock, "ERROR: stack is empty", sizeof("ERROR: stack is empty"), 0) == -1)
                    perror("send failed\n");
                // pthread_mutex_unlock(&my_mutex);
                for(int k=0; k<15;k++){
                    if(k!= index && sockets[i] != -1){
                        fcntl(sockets[k], F_UNLCK, &lock);
                    }
                }
                continue;
            }
            (*index_s)-=2;
            while(*index_s != 0 && head[*index_s] != '\0'){
                (*index_s)--;
            }
            if(head[*index_s] == '\0'){
                (*index_s)++;
            }
            if (send(sock, "sent", sizeof("sent"), 0) == -1)//confirm it worked
                perror("send failed\n");
            // pthread_mutex_unlock(&my_mutex);
            for(int k=0; k<15;k++){
                if(k!= index && sockets[i] != -1){
                    fcntl(sockets[k], F_UNLCK, &lock);
                }
            }
            continue;
        }
        else if(strcmp(command, "TOP\n")==0){
            if(*index_s == 0){
                if (send(sock, "ERROR: stack is empty", sizeof("ERROR: stack is empty"), 0) == -1)
                    perror("send failed\n");
                // pthread_mutex_unlock(&my_mutex);
                for(int k=0; k<15;k++){
                    if(k!= index && sockets[i] != -1){
                        fcntl(sockets[k], F_UNLCK, &lock);
                    }
                }
                continue;
            }
            memset(data, 0, 1024);
            char* temp = top(head + *index_s, bottom);
            int p = 0;
            while(*temp != '\0'){
                data[p] = *temp;
                temp++;
                p++;
            }
            if (send(sock, data, sizeof(data), 0) == -1)//send the data that the user asked for
                perror("send failed\n");
            // pthread_mutex_unlock(&my_mutex);
            for(int k=0; k<15;k++){
                if(k!= index && sockets[i] != -1){
                    fcntl(sockets[k], F_UNLCK, &lock);
                }
            }
            continue;
        }
        else if(strcmp(command, "EXIT")==0){
            // pthread_mutex_unlock(&my_mutex);
            for(int k=0; k<15;k++){
                if(k!= index && sockets[i] != -1){
                    fcntl(sockets[k], F_UNLCK, &lock);
                }
            }
            close(sock);
            pthread_exit(NULL);
        }
        // pthread_mutex_unlock(&my_mutex);
        for(int k=0; k<15;k++){
            if(k!= index && sockets[i] != -1){
                fcntl(sockets[k], F_UNLCK, &lock);
            }
        }
    }
}


int main(void)
{
    head = (char*)malloc(10000000);
    bottom = head;
    index_s = (int*)malloc(1);
    memset(sockets, -1, 15*sizeof(int));
    int sockfd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv, i=0;

        signal(SIGPIPE, SIG_IGN); // on linux to prevent crash on closing socket
    int listeningSocket = -1, data_len = 0;
    char buf[256];
    int flag = 0;
    socklen_t len = sizeof(buf);
    int sock = socket(AF_INET, SOCK_STREAM, 0);//create socket
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    if((listeningSocket = socket(AF_INET , SOCK_STREAM , 0 )) == -1)
    {
        printf("Could not create listening socket : %d" ,errno);
    }
    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    //
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);  //network order

    // Bind the socket to the port with any IP at this port
    if (bind(listeningSocket, (struct sockaddr *)&serverAddress , sizeof(serverAddress)) == -1)
    {
        printf("Bind failed with error code : %d" ,errno);
        // TODO: close the socket
        return -1;
    }
    // printf("Bind() success\n");

    // Make the socket listening; actually mother of all client sockets.
    if (listen(listeningSocket, 1) == -1) //1 is a Maximum size of queue connection requests
                                                                                        //number of concurrent connections 
    {
        printf("listen() failed with error code : %d",errno);
        // TODO: close the socket
        return -1;
    }
    //Accept and incoming connection
    // printf("Waiting for incoming TCP-connections...\n");

    struct sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        sockets[i] = accept(listeningSocket, (struct sockaddr *)&their_addr, &sin_size);
        if (sockets[i] == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        // printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(listeningSocket); // child doesn't need the listener
            start_process(&(sockets[i]), i);
            close(sockets[i]);
            exit(0);
        }
        close(sockets[i]);  // parent doesn't need this
        sockets[i] = -1;
        i=(i+1)%15;
    }
    free(head, 10000000);
    free(index_s, 1);
    return 0;
}
                                        