#include "start_stop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>
#include <stdbool.h>


int server_fd=-1;
volatile int stop_server_flag = 0;
pthread_t accept_thread; //thread pentru acceptarea conexiunilor pentru a separa thread ul principal de acceptarea noilor conexiuni


bool check_password(){
    char input[256];
    printf("Enter administrator password: ");
    fgets(input,sizeof(input),stdin);
    input[strcspn(input,"\n")]=0; //sterge caracterul newline
    if(strcmp(input,ADMIN_PASSWORD)==0){
        return true;
    }else{
        printf("Incorrect passwd!\n");
        return false;
    }
}

void* accept_connections(void* arg) {
    int client_sock = 0;

    while (!stop_server_flag) {  // Așteptăm oprirea serverului
       printf("Waiting for a client connection...\n");
        client_sock = accept(server_fd, 0, 0);
        
        if (client_sock < 0) {
            if (stop_server_flag) break;  // Dacă serverul este oprit, ieșim
            perror("Accept failed");
            continue;
        }
        printf("Client connected!\n");
        // Dacă acceptarea conexiunii a avut succes, procesăm clientul pe un thread
        pthread_mutex_lock(&(server_threadpool->mutex_locker));  // Blocăm mutex-ul pentru a proteja zona critică
        // Procesăm clientul fără a folosi coada
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, (void*)&client_sock) != 0) {
            perror("Failed to create client handler thread");
            close(client_sock);
        } else {
            pthread_detach(client_thread);  // Detachăm thread-ul pentru a nu fi nevoie să-l așteptăm
        }
        pthread_mutex_unlock(&(server_threadpool->mutex_locker));  // Eliberăm mutex-ul
    }

    printf("Accept connections stopped\n");
    return NULL;
}
int start_server() {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("Deschidere server\n");

    server_threadpool = initialize_new_threadpool(NO_THREADS);
    if (!server_threadpool) {
        perror("Failed to initialize threadpool");
        return -1;
    }

    server_threadpool->is_running = true;
    printf("Server initializat cu succes\n");

    // Creare socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY
    };

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        close(server_fd);
        return -1;
    }

    printf("Server running on http://localhost:%d\n", PORT);

    // Creare thread pentru a monitoriza input-ul
    pthread_t input_thread;
    if (pthread_create(&input_thread, NULL, monitor_input, NULL) != 0) {
        perror("Failed to create input thread");
        close(server_fd);
        return -1;
    }else{
        printf("Input monitoring thread created successfully\n");
    }

    // Creare thread pentru a accepta conexiunile
    pthread_t accept_thread;
    if (pthread_create(&accept_thread, NULL, accept_connections, NULL) != 0) {
        perror("Failed to create accept thread");
        close(server_fd);
        return -1;
    }

    // Așteptăm terminarea thread-ului acceptor
    pthread_join(accept_thread, NULL);
    return 0;
}

void stop_server() {
    printf("Shutting down server...\n");

    // Setează flag-ul pentru oprirea serverului
    stop_server_flag = 1;
    printf("stop_server_flag set to 1\n");
    // Așteptăm ca thread-ul care acceptă conexiuni să termine
    if (server_threadpool != NULL) {
    printf("Waiting for server thread to finish...\n");
    pthread_join(server_threadpool->thread, NULL);
    printf("Server thread finished.\n");
    }

    // Închidem socketul serverului
    close(server_fd);
    printf("Server stopped\n");
    kill(getpid(),SIGTERM);
}

void handle_sigterm(int sig) {
    printf("Received SIGTERM. Stopping server...\n");
    stop_server();  // Apelează funcția care oprește serverul
}

void* monitor_input(void* arg) {
    fd_set readfds;
    struct timeval timeout;
    setvbuf(stdout, NULL, _IONBF, 0);

    while (!stop_server_flag) {
        FD_ZERO(&readfds);  // Resetează setul de fișiere citibile
        FD_SET(STDIN_FILENO, &readfds);  // Adaugă stdin la setul de fișiere citibile

        timeout.tv_sec = 1;  // Timeout de 1 secundă
        timeout.tv_usec = 0;

        // Verificăm dacă stdin are date de citit
        int activity = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0) {
            perror("select error");
            continue;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char input[256];
            printf("Enter 'stop' to shut down the server: ");
            fflush(stdout);  // Asigură-te că promptul este afișat imediat

            if (fgets(input, sizeof(input), stdin) != NULL) {
                input[strcspn(input, "\n")] = 0;  // Elimină caracterul de newline

                if (strcmp(input, "stop") == 0) {
                    printf("Închidere server...\n");
                    stop_server();  // Oprește serverul
                }
            }
        }
        // Delay pentru a reduce utilizarea CPU
        usleep(500000);  // Pauză de 0.5 secunde
    }

    return NULL;
}
