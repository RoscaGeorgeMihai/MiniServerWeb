#ifndef CLIENTS_HANDLERS_
#define CLIENTS_HANDLERS_

#define BUFFER_SIZE 8192

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
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

const char *get_file_extension(const char *filename);
const char *get_mime_type(const char *file_ext);
char *url_decode(const char *src);
void build_http_ok(const char *file_name, const char *file_ext, char *response, size_t *response_len);
void build_http_error(const char *file_name, char *response, size_t *response_len);
void build_http_response(const char *file_name, const char *file_ext,
                         char *response, size_t *response_len);
void *handle_client(void *arg);
void process_post_request(const char *buffer,int client_fd);
void process_put_request(const char *buffer,int client_fd);
void interpretPHP(const char *file_name);
void interpretJAVA(const char *file_name);
void extract_file_from_zip(const char *zip_file, char *buffer, size_t *buffer_len,char *mime_type);

#endif