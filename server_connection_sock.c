#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <regex.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "handle.c"

#define BUFFER_SIZE 8192

void* handle_client_request(void *arg);
void http_response(const char *file_name, const char *file_ext, char *response, size_t *response_len);
char *decode(const char* url);
const char *get_file_extension(const char *filename);
const char *get_mime_type(const char *file_ext);

int server_socket;
int PORT = 8080;
struct sockaddr_in server_address;

void assign_address(void){
   server_address.sin_family = AF_INET;
   server_address.sin_addr.s_addr = INADDR_ANY;
   server_address.sin_port = htons(PORT);
}

int main(void){
   server_socket = socket(AF_INET, SOCK_STREAM, 0);
   if (server_socket < 0){
      perror("Socket Failed");
      exit(EXIT_FAILURE);
   }
   assign_address();

   if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0){
      perror("Bind failed.");
      exit(EXIT_FAILURE);
   }

   if (listen(server_socket, 10) < 0){
      perror("Listen error.");
      exit(EXIT_FAILURE);
   }

   while(1){
      struct sockaddr_in client_address;
      socklen_t client_address_length = sizeof(client_address);
      int *client_socket = (int *)malloc(sizeof(int));

      if ((*client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length))< 0){
         perror("Client connection failed.");
      }
      pthread_t thread_id;
      pthread_create(&thread_id, NULL, handle_client_request, (void *)client_socket);
      pthread_detach(thread_id);

   }
   return 0;
}

void* handle_client_request(void *arg){
   int client_socket = *((int *)arg);
   char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));
   
   ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
   if (bytes_received > 0){
      regex_t regex;
      regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
      regmatch_t matches[2];

      if (regexec(&regex, buffer, 2, matches, 0) == 0){
         buffer[matches[1].rm_eo] = '\0';
         const char *url_file_name = matches[1].rm_so + buffer;
         char *file_name = decode(url_file_name);
         if(file_name){curl_free(file_name);}
         char file_ext[32];
         strcpy(file_ext, get_file_extension(file_name));

         char *response = (char *)malloc(BUFFER_SIZE * 2 * sizeof(char));
         size_t response_len;
         http_response(file_name, file_ext, response, &response_len);

         send(client_socket, response, response_len, 0);
         free(response);
         free(file_name);
      }
      regfree(&regex);
   }
   close(client_socket);
   free(arg);
   free(buffer);
}

void http_response(const char *file_name, const char *file_ext, char *response, size_t *response_len){
   const char *mime_type = get_mime_type(file_ext);
   char *header = (char *)malloc(BUFFER_SIZE * sizeof(char));
   snprintf(header, BUFFER_SIZE, 
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: %s\r\n"
      "\r\n",
      mime_type);
   
   int file_ad = open(file_name, O_RDONLY);
   if (file_ad == -1){
      snprintf(response, BUFFER_SIZE,
         "HTTP/1.1 404 Not Found\r\n"
         "Content-Type: text/plain\r\n"
         "\r\n"
         "404 Not Found");
      *response_len = strlen(response);
      return;
   }

   struct stat file_stat;
   fstat(file_ad, &file_stat);
   off_t file_size = file_stat.st_size;

   *response_len = 0;
   memcpy(response, header, strlen(header));
   *response_len = strlen(header);

   ssize_t bytes_read;
   while ((bytes_read = read(file_ad, response + *response_len, BUFFER_SIZE - *response_len)) > 0){
      *response_len += bytes_read;
   }
   free(header);
   close(file_ad);

   
}

