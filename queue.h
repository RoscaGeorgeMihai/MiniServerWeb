#ifndef QUEUE_H
#define QUEUE_H
#include <stdbool.h>
#include <pthread.h>

//definirea unui nod din coada noastra de clienti care urmeaza sa fie gestionati
typedef struct client_node{
    struct client_node* next; //clientul urmator din coada
    int* client_socket; //socket-ul clientului curent
}client_node;

//coada propriu-zisa de clienti
typedef struct clients_queue{
    struct client_node* head; 
    struct client_node* tail;
}clients_queue;

//adauga un client nou la capatul cozii
void push(clients_queue *queue,int client_socket);

//verifica daca coada este goala
bool is_empty(clients_queue* queue_to_check);

//scoate primul element din coada de clienti care asteapta
int pop(clients_queue *queue);

//creeaza un nou nod pentru coada de clienti
client_node* get_new_node(int client_socket);

//returneaza o noua coada de clienti care este initial goala
clients_queue* get_new_queue();

#endif