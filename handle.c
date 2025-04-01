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
   char* decoded = "";
   curl_global_init(CURL_GLOBAL_DEFAULT);
   if(curl){
      decoded = curl_easy_unescape(curl, decoded, 0, &decoded_length);
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
   }
}

const char *get_file_extension(const char *filename) {
   const char *dot = strrchr(filename, '.');
   if(!dot || dot == filename) return "";
   return dot + 1;
}

const char *get_mime_type(const char *file_ext){
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
   }
}



