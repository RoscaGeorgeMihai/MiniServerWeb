#define _GNU_SOURCE
#include "clientshandler.h"
#include <sys/types.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int handle_get_request(char* file_path, int* client_fd){
    int file_fd = open(file_path, O_RDONLY);
    if (file_fd < 0) {
        // 404 - fisierul nu e gasit
        const char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
                                "<html><body><h1>404 Not Found</h1></body></html>";
        send(*(int*)client_fd, not_found, strlen(not_found), 0);
    } else {
        // dimensiune fisier
        off_t file_size = lseek(file_fd, 0, SEEK_END);
        lseek(file_fd, 0, SEEK_SET);

        //  200 OK  - fisier gasit
        char header[512];
        snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", file_size);
        send(*(int*)client_fd, header, strlen(header), 0);

        // trimite continutul fisierului
        sendfile(*(int*)client_fd,file_fd,0,file_size);
    }
}

void* handle_client(void* client_fd)
{
    printf("Handling client %d on thread %d\n",*(int*)client_fd,gettid());
            // Primeste o cerere HTTP 
        char buffer[BUFFER_SIZE] = {0};
        ssize_t bytes_received = recv(*(int*)client_fd, buffer, BUFFER_SIZE, 0);//primeste data de la client si le stocheaza in buffer
        if (bytes_received < 0) {
            perror("Receive failed");
            close(*(int*)client_fd);
        }
    if (strncmp(buffer, "GET", 3) == 0) {
            // Extrage calea fisierului
            char *file_path = buffer + 5;
            char *end_of_path = strchr(file_path, ' ');
            if (end_of_path) {
                *end_of_path = '\0';  // adauga terminatorul null la finalul caii
            }

            // Deschide fisierul

            handle_get_request(file_path,(int*)client_fd);
    } else {
        // 400 - nu e cerere de tip GET
        const char *bad_request = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n"
                                  "<html><body><h1>400 Bad Request</h1></body></html>";
        send(*(int*)client_fd, bad_request, strlen(bad_request), 0);
    }
    close(*(int*)client_fd);

    printf("Handling completed\n");
    return NULL;
}