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
    chdir("./resources");
    load_password_status();
    while (1) {
        int password_check = check_password(); 

        if (password_check == 1) {
            start_server();  
        } else if (password_check == -1) {
            stop_server();  
            break;  
        }
        
    }
}