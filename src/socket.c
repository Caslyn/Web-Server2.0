#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "headerfile.h"

#define BACKLOG 10
#define PORT "5000" // the port clients will be connecting to

static void *get_client_addr(struct sockaddr *);
static struct addrinfo hints, *servinfo, *p;
static struct sockaddr_storage client_addr; // clients address information
static socklen_t sin_size;

// get sockadr, IPv4 or IPv6:
static void *get_client_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
     return &(((struct sockaddr_in*) sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

static void sigchld_handler(int s) {
  // waitpid() might overwrite errno, so we save and restor it:
  int saved_errno = errno;
  while(waitpid(-1, NULL, WNOHANG) > 0);

  errno = saved_errno;
}

int create_socket(int *sockfd){
  int yes = 1, rv;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL,  PORT, &hints, &servinfo)) != 0) {
     fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
     exit(0);
  }

  // loop through all the results and bind the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
     if((*sockfd  = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        perror("server: socket");
        continue;
     }

     if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
       close(*sockfd);  
       perror("setsockopt");
         continue;
     }

     if (bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(*sockfd);
        perror("server:bind");
        continue;
     }
     break;
  }
  freeaddrinfo(servinfo); // all done with this structure

  if (p == NULL){
     fprintf(stderr, "server:failed to bind\n");
     exit(1);
     return -1;
  }
  return sockfd;
}

int begin_listening(int sockfd){
  struct sigaction sa; // signal struct

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    return -1;
  }

  sa.sa_handler = sigchld_handler; // reap zombie processes
  sigemptyset(&sa.sa_mask); // initiazes the signal to be empty
  sa.sa_flags = SA_RESTART; // restart the system call if possible

  if(sigaction(SIGCHLD, &sa, NULL) == -1) {
     perror("sigaction");
     return -1;
  }

  printf("server: waiting for connection...\n");
  return 0;
}

int accept_connection(int sockfd) {
  int new_fd, pid;
  char s[INET6_ADDRSTRLEN];

  while(1) {
     sin_size = sizeof(client_addr);
     new_fd = accept(sockfd, (struct sockaddr *) &client_addr, &sin_size);

     if (new_fd == -1) {
        perror("accept");
        return -1;
     }

     inet_ntop(client_addr.ss_family, get_client_addr((struct sockaddr *) &client_addr), s, sizeof(s));
     printf("server: got connection from %s\n", s);

     if ((pid = fork()) != 0) { // this is the child process//     
       printf("Spawning Child %d\n", pid);
       close(sockfd); // child doesn't need the listening socket
       serve_request(new_fd);
       if (serve_request(new_fd) == -1) {
          perror("send");
          close(new_fd);
          return -1;
       }
       exit(0);
     } else {
       close(new_fd); // parent doesn't need the new fd
     }
  }
  return 0;
}
