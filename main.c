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
#define BUFFER_SIZE 1024

void* handle_client(void* client_fd)
{
    printf("Handling client %d on thread %d\n",*(int*)client_fd,gettid());
            // Primeste o cerere HTTP 
        char buffer[BUFFER_SIZE] = {0};
        ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);//primeste data de la client si le stocheaza in buffer
        if (bytes_received < 0) {
            perror("Receive failed");
            close(client_fd);
        }
if (strncmp(buffer, "GET", 3) == 0) {
            // Extrage calea fisierului
            char *file_path = buffer + 5;
            char *end_of_path = strchr(file_path, ' ');
            if (end_of_path) {
                *end_of_path = '\0';  // adauga terminatorul null la finalul caii
            }

            // Deschide fisierul
            int file_fd = open(file_path, O_RDONLY);
            if (file_fd < 0) {
                // 404 - fisierul nu e gasit
                const char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
                                        "<html><body><h1>404 Not Found</h1></body></html>";
                send(client_fd, not_found, strlen(not_found), 0);
            } else {
                // dimensiune fisier
                off_t file_size = lseek(file_fd, 0, SEEK_END);
                lseek(file_fd, 0, SEEK_SET);

                //  200 OK  - fisier gasit
                char header[512];
                snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", file_size);
                send(client_fd, header, strlen(header), 0);

                // trimite continutul fisierului
                sendfile(client_fd, file_fd, 0, file_size);
                close(file_fd);
            }
        } else {
            // 400 - nu e cerere de tip GET
            const char *bad_request = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n"
                                      "<html><body><h1>400 Bad Request</h1></body></html>";
            send(client_fd, bad_request, strlen(bad_request), 0);
        }
    close(*(int*)client_fd);

    printf("Handling completed\n");
    return NULL;
}

void main(){
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Deschidere server\n");

    server_threadpool=initialize_new_threadpool(NO_THREADS);
    printf("Server initializat cu succes\n");

    setvbuf(stdout, NULL, _IONBF, 0);  //outputul este scris diirect in terminal

    // Creare socket = specifica familia de adrese(IPv4),tipul de socket(TCP),permite sistemului sa aleaga protocolul pentru SOCK_STREAM(TCP)
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Setam optiunile socketului ca sa evitam resetarea conexiunii
    struct timeval timeout;
    timeout.tv_sec = 30; //timp maxim de asteptare de 30s pentru raspunsul de la socket
    timeout.tv_usec = 0; // nu avem timp de asteptare suplimentar
    if (setsockopt(server_fd, SOL_SOCKET, SO_RCVBUF, (char *)&timeout, sizeof(timeout)) < 0) { //seteaza un timeout pentru bufferul de receptie al socketului
        perror("Failed to set socket options");
        close(server_fd);
        return 1;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY
    };

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { //asociaza un socket cu o adresa si un port, socketul asculta pe adresa ip si pe port
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 10) < 0) {//serverul asteapta conexiuni
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    printf("Server running on http://localhost:%d\n", PORT);

    int client_sock = 0;
    while(true){
        client_sock=accept(server_fd,0,0);

        pthread_mutex_lock(&(server_threadpool->mutex_locker)); //<-sectiune critica => trebuie blocata folosind mutex-ul pentru a nu efectua 
        push(server_threadpool->clients,client_sock); //doua thread-uri operatiuni simultane pe acceasi coada si a o lasa intr-o stare inconsecventa
        pthread_mutex_unlock(&(server_threadpool->mutex_locker)); 
    }

    close(server_fd);   
    /*TO DO 
        Dezalocarea si curatarea threadpool-ului
    */
    printf("Inchidere server\n");
} 
