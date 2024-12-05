#ifndef CLIENTS_HANDLERS_
#define CLIENTS_HANDLERS_

#define BUFFER_SIZE 4096

#include <stdio.h>     
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>     
#include <sys/types.h>  
#include <sys/stat.h>   
#include <fcntl.h>      
#include <sys/socket.h> 
#include <sys/sendfile.h>
#include <regex.h>      
#include <pthread.h>    
#include <errno.h>      

const char *get_file_extension(const char *filename);
const char *get_mime_type(const char *file_ext);
char *url_decode(const char *src);
void build_http_ok(const char *file_name, const char *file_ext, char *response, size_t *response_len);
void build_http_error(const char *file_name, char *response, size_t *response_len);
void build_http_response(const char *file_name, const char *file_ext,
                         char *response, size_t *response_len);
void *handle_client(void *arg);
void process_post_request(const char *buffer);
void process_put_request(const char *buffer);
void interpretPHP(const char *file_name);
void interpretJAVA(const char *file_name);

#endif