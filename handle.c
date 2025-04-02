#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <curl/curl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <regex.h>
#include <pthread.h>
#include <magic.h>

char* decode(const char *url){
   CURL *curl;
   CURLcode res;
   int decoded_length;
   char* decoded = NULL;
   curl_global_init(CURL_GLOBAL_DEFAULT);
   curl = curl_easy_init();
   if(curl){
      decoded = curl_easy_unescape(curl, url, 0, &decoded_length);
      if(decoded){
         curl_global_cleanup();
         return decoded;
      }else{
         curl_easy_cleanup(curl);
         curl_global_cleanup();
         return NULL; 
      }
   }else{
      curl_global_cleanup();
      return NULL;
   }
}

const char *get_file_extension(const char *filename) {
   const char *dot = strrchr(filename, '.');
   if(!dot || dot == filename) return "";
   return dot + 1;
}

const char *get_mime_type(const char *file_ext){
   if (strcmp(file_ext, "html") == 0 || strcmp(file_ext, "htm") == 0) {
      return "text/html";  
  } else if (strcmp(file_ext, "css") == 0) {
      return "text/css";
  } else if (strcmp(file_ext, "js") == 0) {
      return "application/javascript"; 
  } else if (strcmp(file_ext, "png") == 0) {
      return "image/png"; 
  } else if (strcmp(file_ext, "jpg") == 0 || strcmp(file_ext, "jpeg") == 0) {
      return "image/jpeg"; 
  } else if (strcmp(file_ext, "txt") == 0) {
      return "text/plain";  
  } else if (strcmp(file_ext, "json") == 0) {
      return "application/json"; 
  }
   const char *mime;
   magic_t magic;
   magic = magic_open(MAGIC_MIME_TYPE);
   magic_load(magic, NULL);
   magic_compile(magic, NULL);
   mime = magic_file(magic, file_ext);
   if(mime){
      magic_close(magic);
      return mime;
   }else{
      magic_close(magic);
      return "application/octet-stream";
   }
}

void send_error_response(int client_socket) {
   const char *error_response = "HTTP/1.1 404 Not Found\r\n"
                                "Content-Type: text/plain\r\n"
                                "\r\n"
                                "404 Not Found";
   send(client_socket, error_response, strlen(error_response), 0);
}


