#include "threadpool.h"
#include <stdbool.h>
#include <stdlib.h>

void* thread_function(void* ){
    while(true){
        if(is_empty(server_threadpool->clients))
            continue;

        pthread_mutex_lock(&(server_threadpool->mutex_locker));
        int client_socket=pop(server_threadpool->clients); //zona critica => trebuie blocata folosind mutex-uri
        pthread_mutex_unlock(&(server_threadpool->mutex_locker));
        
        if(client_socket==0)
            continue;
        handle_client(&client_socket);
        
    }
    return NULL;
}

threadpool* initialize_new_threadpool(size_t no_of_threads){
    server_threadpool = (threadpool*)malloc(sizeof(threadpool));
    server_threadpool->clients=get_new_queue();
    
    pthread_mutex_init(&server_threadpool->mutex_locker,NULL); //initializarea mutex-ului folosit la blocarea thread-urilor

    server_threadpool->number_of_threads=no_of_threads;
    server_threadpool->threads_availabe=(pthread_t*)malloc(sizeof(pthread_t)*no_of_threads); //vector de thread-uri care vor fi puse la dispozitia server-ului
    for(size_t i=0;i<no_of_threads;i++){
        pthread_create(&server_threadpool->threads_availabe[i],NULL,thread_function,NULL); //crearea propriu zisa a thread-urilor si atribuirea lor unui job (functie)
    }
    return server_threadpool;
}