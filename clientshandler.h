#ifndef CLIENTS_HANDLERS_
#define CLIENTS_HANDLERS_

#define BUFFER_SIZE 1024

void* handle_client(void* client_fd);
int handle_get_request(char* file_path, int* client_fd);
int handle_post_request(char* body,int*client_fd);

#endif