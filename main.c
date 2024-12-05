#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h> // pentru struct sockaddr_in
#include <string.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include "threadpool.h"
#include "clientshandler.h"
#include"start_stop.h"

int main() {
    while (1) {
        int password_check = check_password();  // Cerem parola

        if (password_check == 1) {
            start_server();  // Dacă parola este corectă, pornește serverul
        } else if (password_check == -1) {
            stop_server();  // Dacă utilizatorul scrie "stop", oprește serverul
            break;  // Oprim execuția programului
        }
        
    }
}