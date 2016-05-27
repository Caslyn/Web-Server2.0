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
  printf("%p is reading socket  %d\n", pthread_self(), sockfd);
  int rc;
  req *req= malloc(sizeof(req));

  if((rc = read_request(sockfd, req)) <= 0) { // quit if we didn't read any bytes
    printf("Couldn't read any bytes\n");
    free(req->content);
    free(req);
    return -1;
  } else {
    printf("%s\n", req->content);
    parse_headers(req); 
    write_response(sockfd, req);
    // clean up
    free(req->url);
    free(req->content);
    free(req);
  }
  return 0;
}

int read_request(int sockfd, req *req){
   int bytes_recv, cp = 0;
   char content_buf[CONTENT_LEN];
   size_t initial_len = CONTENT_LEN;
   req->content = (char *) calloc(1, initial_len);
   poll_wait(sockfd, POLLIN);
   while ((bytes_recv = recv(sockfd, content_buf, CONTENT_LEN, 0)) > 0) {
     printf("%d bytes have been read\n", bytes_recv);
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
   return bytes_recv;
}

int parse_headers(req *req) {
     char *s = req->content, *e;

     while(*s++ && !isspace(*s)); // skip over method
     e = (s+=2); // skip over ' ' & '/' 
     while(*e++ && !isspace(*e)); //capturing url
     req->url = calloc(1, e - s);
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
  struct stat st;
 
  char real_path[HEADER_LEN];
  realpath(req->url, real_path);
  printf("URL: %s\n", real_path);

  if ((file = open(real_path, O_RDONLY)) < 0) {
    printf("File Not Found\n\n");
    file = open("notfound.html", O_RDONLY);
    fstat(file, &st);
    off_t size = st.st_size;  
    send_response_headers(sockfd, size, "404 Not Found", get_file_type(req->url));
    send_file(file, sockfd, size, req->url);
  } else {
    printf("Sending Request File\n");
    fstat(file, &st);
    off_t size = st.st_size;  
    send_response_headers(sockfd, size, "200 OK", get_file_type(req->url));
    send_file(file, sockfd, size, req->url); }
  return 0;
}

int send_file(int file, int sockfd, off_t original_size, char *ext) {
   off_t offset = 0;
   int complete = FALSE;
   off_t size = original_size;

   while(complete == FALSE) {
      if (sendfile(file, sockfd, offset, &size, 0,0) < 0) {
         if (errno == EWOULDBLOCK) {
           printf("Wrote %zd bytes to socket\n", size);
           offset+=size;
           size = original_size; // reset size to original size, since the num of bytes sent is stored in this param
           poll_wait(sockfd, POLLOUT | POLLWRBAND);
           continue;
         } else {
          perror("sendfile");
          return 1;
         }
      } else {
        complete = TRUE;
      }
   }
   close(file);
   return offset;
}

char *get_file_type(char *url) {
  char *ext = strchr(url, '.');
  if (!strcmp(ext, ".png")) {
   return "image/png";
  } else {
  return "text/html";
  }
}

void send_response_headers(int sockfd, off_t content_size, char *status, char *ext) {
  char header_buf[HEADER_LEN];
  sprintf(header_buf, "HTTP/1.1 %s\nContent-Type:%s\nContent-Length:%lld\n\n", status, ext, content_size);
  printf("%s", header_buf);
  send(sockfd, header_buf, strlen(header_buf), 0);
}
