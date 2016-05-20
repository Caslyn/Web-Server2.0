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
  parse_headers(req); 
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

     printf("%s", req->content);
     return 0;
 }
 
int parse_headers(req *req) {
     char *s = req->content, *e;

     while(*s++ && !isspace(*s)); // skip over method
     s+=2;
     e = s;
     while(*e++ && !isspace(*e)); //capturing url
     req->url = malloc(e - s);

     if (req->url == NULL) {
        // memory allocation failure
        free(req->url);
        return -1;
     }

     memcpy(req->url, s, e-s);
     return 0;
 }
  int write_response(int sockfd, req *req){
    int file;
    char real_path[HEADER_LEN];
    printf("req->url: %s\n", req->url);
    realpath(req->url, real_path);
    printf("URL: %s\n", real_path);

    if ((file = open(real_path, O_RDONLY)) >= 0) {
      if (!send_file(file, sockfd)) {
       printf("Sent Requested File\n");
       return 0;
      }
    } else {
      printf("File Not Found\n");
      file = open("notfound.html", O_RDONLY);
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
