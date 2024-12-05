#ifndef START_STOP_H
#define START_STOP_H


#include <stdio.h>
#include <stdbool.h>
#include "threadpool.h"
#include "clientshandler.h"

#define PORT 8080
#define NO_THREADS 10
#define ADMIN_PASSWORD "server"

extern int server_fd;
extern threadpool *server_threadpool; 
extern volatile int stop_server_flag;
extern pthread_t accept_thread;

bool check_password();
int start_server();
void stop_server();
void* monitor_input(void* arg);
void *accept_connections(void*arg);


#endif //START_STOP_H