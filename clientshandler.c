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
int handle_get_request(char* file_path, int* client_fd) {
    int file_fd = open(file_path, O_RDONLY);
    if (file_fd < 0) {
        // 404 - fișierul nu a fost găsit
        const char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n"
                                "<html><body><h1>404 Not Found</h1></body></html>";
        if (send(*(int*)client_fd, not_found, strlen(not_found), 0) < 0) {
            perror("Failed to send 404 response");
            close(*(int*)client_fd);
            return -1;
        }
    } else {
        // dimensiune fișier
        off_t file_size = lseek(file_fd, 0, SEEK_END);
        if (file_size < 0) {
            perror("Failed to seek file");
            close(file_fd);
            return -1;
        }
        lseek(file_fd, 0, SEEK_SET);

        // 200 OK - fișier găsit
        char header[512];
        int header_len = snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %ld\r\n\r\n", file_size);
        if (header_len < 0) {
            perror("Failed to format HTTP header");
            close(file_fd);
            return -1;
        }

        if (send(*(int*)client_fd, header, header_len, 0) < 0) {
            perror("Failed to send HTTP header");
            close(file_fd);
            return -1;
        }

        // trimite conținutul fișierului
        if (sendfile(*(int*)client_fd, file_fd, 0, file_size) < 0) {
            perror("Failed to send file");
            close(file_fd);
            return -1;
        }
    }

    close(file_fd);
    return 0;
}
int handle_post_request(char* body, int* client_fd) {
    printf("Received POST data: %s\n", body);
    int file_fd = open("received_data.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd < 0) {
        perror("Error opening file for POST data");
        const char *server_error = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\n"
                                   "<html><body><h1>500 Internal Server Error</h1></body></html>";
        if (send(*(int*)client_fd, server_error, strlen(server_error), 0) < 0) {
            perror("Failed to send 500 response");
        }
        return -1;
    }

    // Scrierea datelor în fișier
    if (write(file_fd, body, strlen(body)) < 0) {
        perror("Error writing POST data to file");
        const char *server_error = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\n\r\n"
                                   "<html><body><h1>500 Internal Server Error</h1></body></html>";
        if (send(*(int*)client_fd, server_error, strlen(server_error), 0) < 0) {
            perror("Failed to send 500 response");
        }
        close(file_fd);
        return -1;
    }

    close(file_fd);

    // Răspuns de succes
    const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                           "<html><body><h1>Data Received Successfully</h1></body></html>";
    if (send(*(int*)client_fd, response, strlen(response), 0) < 0) {
        perror("Failed to send success response");
        return -1;
    }

    return 0;
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
    }else if(strncmp(buffer, "POST", 4) == 0){
        char *body_start = strstr(buffer, "\r\n\r\n");  // cauta inceputul corpului
        if (body_start) {
            body_start += 4;  // sarim de "\r\n\r\n"
            handle_post_request(body_start, (int*)client_fd);
        } else {
            const char *bad_request = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n"
                                      "<html><body><h1>400 Bad Request</h1></body></html>";
            send(*(int*)client_fd, bad_request, strlen(bad_request), 0);
        }
    }
     else {
        // 400 - nu e cerere de tip GET
        const char *bad_request = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n"
                                  "<html><body><h1>400 Bad Request</h1></body></html>";
        send(*(int*)client_fd, bad_request, strlen(bad_request), 0);
    }
    close(*(int*)client_fd);

    printf("Handling completed\n");
    return NULL;
}