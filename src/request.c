#include <ctype.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "headerfile.h"

int serve_request(int sockfd)
{
  req *req= malloc(sizeof(req));

  read_request(sockfd, req);
  write_response(sockfd, req);

   free(req->url);
   free(req->content);
   free(req);
   return 0;
}

int read_request(int sockfd, req *req){
     int bytes_recv, cp = 0;
     char content_buf[CONTENT_LEN];
     size_t chunk_size = CONTENT_LEN; //TODO: understand what size_t is
     req->content = malloc(chunk_size);

     while ((bytes_recv = recv(sockfd, content_buf, CONTENT_LEN, 0)))  {
       if (bytes_recv + cp > chunk_size) { // check if we have run out room in our content buffer
         chunk_size *= 2; // double size
         char *tmp = realloc(req->content, chunk_size);
         if (tmp) {
            req->content = tmp; 
         } else {
            // memory allocation failure
            free(req->content);
            cp = 0;
            break;
         }
       }
       memcpy(req->content + cp, content_buf, bytes_recv);
     }

     parse_headers(req); // parse request, passing in content
     printf("%s", req->content);
     return 0;
 }
 
int parse_headers(req *req) {
     char *s = req->content, *e;

     while(s++ && !isspace(*(req->content++))); // skip over method
     e = s;
     while(e++ && !isspace(*(req->content++))); //capturing url
     req->url = malloc(e - s + 1);

     if (req->url == NULL) {
        // memory allocation failure
        free(req->url);
     }

     memcpy(req->url, s, e-s);
     printf("URL: %s\n", req->url);
     return 0;
 }
  int write_response(int sockfd, req *request){
       int file;
       char actualpath[HEADER_LEN] = "../assets/";
       strcat(actualpath, request->url);
        char path[HEADER_LEN];

        char *pathptr = realpath(actualpath, path);
        printf("Locating File: %s\n", actualpath);
        if ((file = open(pathptr, O_RDONLY)) >= 0) {
          if (!send_file(file, sockfd)) {
           return 0;
          }
        } else {
          printf("File Not Found\n");
          file = open("../assets/notfound.html", O_RDONLY);
          send_file(file, sockfd);
        }
        return 0;
  }

int send_file(int file, int sockfd) {
       struct stat st;
       fstat(file, &st);
       off_t offset = 0, size = st.st_size;
       if (sendfile(file, sockfd, offset, &size, 0,0) < 0) {
           printf("Error Sending File\n");
           return -1;
       }
       close(file);
       return 0;
}
