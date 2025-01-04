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
#include<stdbool.h>

#define PORT_FILE "port_config.txt"
#define PASSWORD_STATUS_FILE "password_status.txt"
volatile bool is_password_enabled = true; 
volatile int max_clients = 10;  // Numărul maxim de clienți acceptați simultan


int server_fd=-1;
volatile int stop_server_flag = 0;
pthread_t accept_thread; //thread pentru acceptarea conexiunilor pentru a separa thread ul principal de acceptarea noilor conexiuni


void load_password_status() {
    FILE *file = fopen(PASSWORD_STATUS_FILE, "r");
    if (!file) {
        printf("Password status file not found. Defaulting to enabled.\n");
        is_password_enabled = true;  // Parola este activată implicit
        return;
    }

    char status[16];
    if (fgets(status, sizeof(status), file)) {
        if (strncmp(status, "enabled", 7) == 0) {
            is_password_enabled = true;
        } else if (strncmp(status, "disabled", 8) == 0) {
            is_password_enabled = false;
        } else {
            printf("Invalid password status in file. Defaulting to enabled.\n");
            is_password_enabled = true;
        }
    } else {
        printf("Password status file is empty. Defaulting to enabled.\n");
        is_password_enabled = true;
    }

    fclose(file);
}

void save_password_status() {
    FILE *file = fopen(PASSWORD_STATUS_FILE, "w");
    if (!file) {
        perror("Failed to open password status file for writing");
        return;
    }

    fprintf(file, "%s\n", is_password_enabled ? "enabled" : "disabled");
    fclose(file);
}

void toggle_password() {
    is_password_enabled = !is_password_enabled;
    save_password_status();

    if (is_password_enabled) {
        printf("Password protection enabled.\n");
    } else {
        printf("Password protection disabled.\n");
    }
}

// Funcție pentru citirea portului din fișier
int read_port_from_file() {
    FILE *file = fopen(PORT_FILE, "r");
    if (file == NULL) {
        perror("Failed to open port file. Using default port.");
        stop_server();
        return 8080; // Port implicit
    }
    int port;
    if (fscanf(file, "%d", &port) != 1) {
        perror("Failed to read port from file. Using default port.");
        stop_server();
    }
    fclose(file);
    return port;
}

// Funcție pentru scrierea portului în fișier
void write_port_to_file(int port) {
    FILE *file = fopen(PORT_FILE, "w");
    if (file == NULL) {
        perror("Failed to open port file for writing.");
        return;
    }
    fprintf(file, "%d\n", port);
    fclose(file);
    printf("Port saved to file: %d\n", port);
}

bool check_password() {
    if (!is_password_enabled) {
        return true;  // Dacă parola este dezactivată, accesul este permis
    }

    char input[256];
    printf("Enter administrator password: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;  // Eliminăm caracterul newline
    if (strcmp(input, ADMIN_PASSWORD) == 0) {
        return true;
    } else {
        printf("Incorrect password!\n");
        return false;
    }
}

void restart_server_on_new_port(int new_port) {
    printf("Reconfiguring server to use port %d...\n", new_port);

    // Închidem socket-ul curent
    close(server_fd);

    // Creăm un nou socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Failed to create new socket");
        exit(EXIT_FAILURE);  // Oprim serverul dacă nu putem crea un nou socket
    }

    // Configurăm adresa serverului
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(new_port),
        .sin_addr.s_addr = INADDR_ANY
    };

    // Asociem socket-ul cu noul port
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Failed to bind to new port");
        exit(EXIT_FAILURE);
    }

    // Pornim ascultarea pe noul port
    if (listen(server_fd, 10) < 0) {
        perror("Failed to listen on new port");
        exit(EXIT_FAILURE);
    }

    printf("Server reconfigured and now listening on http://localhost:%d\n", new_port);
}

void configure_server() {
    // Verificăm dacă parola este activată
    if (is_password_enabled && !check_password()) {
        return;  // Dacă parola este greșită, ieșim din funcție
    }

    int choice = 0;
    do {
        printf("\n=== Server Configuration Menu ===\n");
        printf("1. Change listening port\n");
        printf("2. %s password protection\n", is_password_enabled ? "Disable" : "Enable");
        printf("3. Exit configuration\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number between 1 and 3.\n");
            while (getchar() != '\n');  // Curățăm buffer-ul
            continue;
        }

        switch (choice) {
            case 1: {
                int new_port;
                printf("Enter new port number: ");
                if (scanf("%d", &new_port) != 1 || new_port < 1 || new_port > 65535) {
                    printf("Invalid port number. Please enter a value between 1 and 65535.\n");
                    while (getchar() != '\n');  // Curățăm buffer-ul
                    continue;
                }
                write_port_to_file(new_port);  // Salvează noul port
                printf("Port changed to %d.\n", new_port);
                restart_server_on_new_port(new_port);  // Aplica schimbarea imediat
                break;
            }
            case 2:
                toggle_password();  // Activează/dezactivează parola
                break;
            case 3:
                printf("Exiting configuration menu.\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    } while (choice != 3);
}


void* accept_connections(void* arg) {
    int client_sock = 0;

    while (!stop_server_flag) {  
       printf("Waiting for a client connection...\n");
        client_sock = accept(server_fd, 0, 0);
        
        if (client_sock < 0) {
            if (stop_server_flag) break;  
            perror("Accept failed");
            continue;
        }
        printf("Client connected!\n");
        pthread_mutex_lock(&(server_threadpool->mutex_locker));  
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, (void*)&client_sock) != 0) {
            perror("Failed to create client handler thread");
            close(client_sock);
        } else {
            pthread_detach(client_thread);  
        }
        pthread_mutex_unlock(&(server_threadpool->mutex_locker)); 
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

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return -1;
    }
    int port=read_port_from_file();
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
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

    printf("Server running on http://localhost:%d\n", port);

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

    pthread_join(accept_thread, NULL);
    return 0;
}

void stop_server() {
    printf("Shutting down server...\n");
    stop_server_flag = 1;
    printf("stop_server_flag set to 1\n");
    if (server_threadpool != NULL) {
    printf("Waiting for server thread to finish...\n");
    pthread_join(server_threadpool->thread, NULL);
    printf("Server thread finished.\n");
    }
    close(server_fd);
    printf("Server stopped\n");
    kill(getpid(),SIGTERM);
}

void handle_sigterm(int sig) {
    printf("Received SIGTERM. Stopping server...\n");
    stop_server();  
}

void* monitor_input(void* arg) {
    fd_set readfds;
    struct timeval timeout;
    setvbuf(stdout, NULL, _IONBF, 0);

    while (!stop_server_flag) {
        FD_ZERO(&readfds);  
        FD_SET(STDIN_FILENO, &readfds);  

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
            fflush(stdout);  // Asigură-te că promptul este afișat imediat

            if (fgets(input, sizeof(input), stdin) != NULL) {
                input[strcspn(input, "\n")] = 0;  // Elimină caracterul de newline

                if (strcmp(input, "stop") == 0) {
                    printf("Închidere server...\n");
                    stop_server();  // Oprește serverul
                } else if (strcmp(input, "configurare") == 0) {
                    configure_server();
                }
            }
        }
        // Delay pentru a reduce utilizarea CPU
        usleep(500000);  // Pauză de 0.5 secunde
    }

    return NULL;
}
