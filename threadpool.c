#include "threadpool.h"
#include <stdbool.h>
#include <stdlib.h>


void* thread_function(void* arg) {
    while (true) {
        pthread_mutex_lock(&(server_threadpool->mutex_locker));
        
        if (server_threadpool->stop) {
            pthread_mutex_unlock(&(server_threadpool->mutex_locker));
            break;  // Ieșim din bucla de lucru a thread-ului
        }

        // Așteaptă un client în coadă
        while (is_empty(server_threadpool->clients)) {
            pthread_cond_wait(&server_threadpool->not_empty_queue, &server_threadpool->mutex_locker);
        }

        int client_socket = pop(server_threadpool->clients);  // Zona critică => trebuie blocată folosind mutex-uri
        if (client_socket == 0) continue;

        if (handle_client(&client_socket) < 0) {
            perror("Error handling client request");
        }
        
        pthread_mutex_unlock(&(server_threadpool->mutex_locker));
    }
    return NULL;
}


threadpool* initialize_new_threadpool(size_t no_of_threads){
    server_threadpool = (threadpool*)malloc(sizeof(threadpool));
    server_threadpool->clients=get_new_queue();
    
    pthread_mutex_init(&server_threadpool->mutex_locker,NULL); //initializarea mutex-ului folosit la blocarea thread-urilor
    pthread_cond_init(&server_threadpool->not_empty_queue,NULL); //initializam o variabila conditionata care sa activeze thread-urile cand in client este introdus in coada de clienti

    server_threadpool->number_of_threads=no_of_threads;
    server_threadpool->threads_availabe=(pthread_t*)malloc(sizeof(pthread_t)*no_of_threads); //vector de thread-uri care vor fi puse la dispozitia server-ului
    for(size_t i=0;i<no_of_threads;i++){
        pthread_create(&server_threadpool->threads_availabe[i],NULL,thread_function,NULL); //crearea propriu zisa a thread-urilor si atribuirea lor unui job (functie)
    }
    return server_threadpool;
}

void destroy_threadpool(threadpool*pool){
    if(pool){
         for (size_t i = 0; i < pool->number_of_threads; i++) {
            pthread_cancel(pool->threads_availabe[i]); // Anulați execuția thread-ului
        }

        // Eliberare resurse
        free(pool->threads_availabe);
        free(pool->clients); // presupunând că clients este alocat dinamic
        pthread_mutex_destroy(&pool->mutex_locker); // Distruge mutex-ul
        pthread_cond_destroy(&pool->not_empty_queue); // Distruge variabila de condiție
        free(pool);
    }
}