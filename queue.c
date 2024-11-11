#include "queue.h"
#include <stdlib.h>

void push(clients_queue *queue,int client_socket){
    client_node* node_to_insert = get_new_node(client_socket);
    if(queue == NULL)
        return;

    if (queue->head == NULL){
        queue->head = node_to_insert;
        queue->tail = node_to_insert;
        return;
    }

    queue->tail->next=node_to_insert;
    queue->tail=node_to_insert;
}

bool is_empty(clients_queue* queue_to_check){
    if(queue_to_check == NULL || queue_to_check->head == NULL)
        return true;
    return false;

}

int pop(clients_queue *queue){
    if (queue == NULL || queue->head == NULL)
        return 0;

    client_node* node_to_pop = queue->head;
    queue->head = queue->head->next;

    if(queue->head == NULL)
        queue->tail = NULL;

    node_to_pop->next = NULL;
    int client_socket = *(node_to_pop->client_socket);
    free(node_to_pop);
    return client_socket;
}

client_node* get_new_node(int client_socket){
    client_node *new_node = (client_node*)malloc(sizeof(client_node));
    new_node->next = NULL;
    new_node->client_socket = (int*)malloc(sizeof(int));
    *new_node->client_socket = client_socket;

    return new_node;
}

clients_queue* get_new_queue(){
    clients_queue* new_queue=(clients_queue*)malloc(sizeof(clients_queue));
    new_queue->head = NULL;
    new_queue->tail = NULL;

    return new_queue;
}