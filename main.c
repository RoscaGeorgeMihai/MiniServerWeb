#define _GNU_SOURCE
#include<sys/socket.h>
#include<sys/types.h>
#include <arpa/inet.h> // pentru struct sockaddr_in
#include<string.h>
#include <fcntl.h>
#include<sys/sendfile.h>
#include<unistd.h>
#include<netinet/in.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include "threadpool.h"

#define PORT 8080
#define NO_THREADS 10

//functia care gestioneaza clientii pe masura ce acestia sunt scosi din coada de asteptare
void* handle_client(void* client_fd)
{
    printf("Handling client %d on thread %d\n",*(int*)client_fd,gettid());

    char buffer[256]={0};
    recv(*(int*)client_fd,buffer,256,0);
    char *f=buffer+5;//pt get
    int opened_fd=open(f,O_RDONLY);
    
    sendfile(*(int*)client_fd,opened_fd,0,256);
    close(opened_fd);
    close(*(int*)client_fd);

    printf("Handling completed\n");
    return NULL;
}


void main(){
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Deschidere server\n");

    server_threadpool=initialize_new_threadpool(NO_THREADS);
    printf("Server initializat cu succes\n");

    int server_socket=0;
    server_socket=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in addr ={
      AF_INET,
       htons(PORT),
       0
    };
    bind(server_socket,&addr,sizeof(addr));
    listen(server_socket,10);

    int client_sock = 0;
    while(true){
        client_sock=accept(server_socket,0,0);

        pthread_mutex_lock(&(server_threadpool->mutex_locker)); //<-sectiune critica => trebuie blocata folosind mutex-ul pentru a nu efectua 
        push(server_threadpool->clients,client_sock); //doua thread-uri operatiuni simultane pe acceasi coada si a o lasa intr-o stare inconsecventa
        pthread_mutex_unlock(&(server_threadpool->mutex_locker)); 
    }

    close(server_socket);   
    /*TO DO 
        Dezalocarea si curatarea threadpool-ului
    */
    printf("Inchidere server\n");
} 