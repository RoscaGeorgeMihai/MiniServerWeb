#ifndef _THREADPOOL_
#define _THREADPOOL_

#include <pthread.h>
#include "queue.h"

//structura care reprezinta threadpool-ul din care server-ul nostru va alege thread-uri pentru a gestiona clientii
typedef struct{
    clients_queue *clients;
    pthread_t *threads_availabe;
    size_t number_of_threads;
    pthread_mutex_t mutex_locker;
    pthread_cond_t not_empty_queue;
    bool stop;
    bool is_running;
    pthread_t thread; 
}threadpool;

static threadpool* server_threadpool;

//initializeaza threadpool-ul server-ului
threadpool* initialize_new_threadpool(size_t no_of_threads);

//functia pe care o va executa fiecare thread din threadpool pentru a verifica daca sunt clienti si a apela functia care ii gestioneaza
void* thread_function(void* );

//functia care gestioneaza clientii de pe socketsi
void* handle_client(void* client_socket);

void destroy_threadpool(threadpool*pool);

#endif