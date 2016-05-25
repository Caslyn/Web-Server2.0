#include <errno.h>
#include <ctype.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <poll.h>
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
   size_t initial_len = CONTENT_LEN;
   req->content = malloc(initial_len);

   while ((bytes_recv = recv(sockfd, content_buf, CONTENT_LEN, 0)) > 0) {
     if (bytes_recv + cp >= initial_len) { // exceed storage allocation capacity
        initial_len *= 2; // double size
        char *tmp = realloc(req->content, initial_len);
        if (tmp) {
           printf("..allocating more storage for request...\n");
           req->content = tmp;
        } else {
          // memory allocation failure
          free(req->content);
          cp = 0;
       }
      memcpy(req->content + cp, content_buf, bytes_recv);
      cp += bytes_recv;
     } else if (bytes_recv < CONTENT_LEN) { // we have recieved the end of the request
      memcpy(req->content + cp, content_buf, bytes_recv);
      break;
     } else {  // we have enough storage but have not recieved the end of the request
      memcpy(req->content + cp, content_buf, bytes_recv);
      cp += bytes_recv;
     }
   }

   printf("%s", req->content);
   return 0;
}

int parse_headers(req *req) {
     char *s = req->content, *e;

     while(*s++ && !isspace(*s)); // skip over method
     e = (s+=2); // skip over ' ' & '/' 
     while(*e++ && !isspace(*e)); //capturing url
     req->url = malloc(e - s);
     if (req->url == NULL) {
        // memory allocation failure
        free(req->url);
        return -1;
     }

     memcpy(req->url, s, e-s);
     req->url[e-s] = '\0';
     return 0;
 }
  int write_response(int sockfd, req *req){
    int file;
    char real_path[HEADER_LEN];
    realpath(req->url, real_path);
    printf("URL: %s\n", real_path);

    if ((file = open(real_path, O_RDONLY)) >= 0) {
      if (!send_file(file, sockfd)) {
       printf("Sent Requested File\n\n");
       return 0;
      }
      printf("File Not Found\n\n");
      file = open("notfound.html", O_RDONLY);
      send_file(file, sockfd);
    }
    return 0;
  }

int send_file(int file, int sockfd) {
   struct stat st;
   fstat(file, &st);
   off_t offset = 0, size = st.st_size;
   int complete = FALSE;

   while(complete == FALSE) {
      if (sendfile(file, sockfd, offset, &size, 0,0) < 0) {
         if (errno == EAGAIN) {
           printf("Wrote %zd bytes to socket\n", size);
           offset+=size;
           poll_wait(sockfd, POLLOUT | POLLERR);
           continue;
         } else {
          perror("sendfile");
          return -1;
         } 
      } else {
        complete = TRUE;
      } 
   }

   close(file);
   return offset;
}
