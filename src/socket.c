#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "headerfile.h"

#define BACKLOG 10
#define PORT "5000" // the port clients will be connecting to

static struct addrinfo *hints, *servinfo, *p;

static void sigchld_handler(int s) {
  // waitpid() might overwrite errno, so we save and restor it:
  int saved_errno = errno;
  while(waitpid(-1, NULL, WNOHANG) > 0);

  errno = saved_errno;
}

int listen_on_socket(int *sockfd){
  int yes = 1;
  struct sigaction sa; // signal struct

  get_serv_addr(hints, servinfo);

  // loop through all the results and bind the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
     if((*sockfd  = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        perror("server: socket");
        continue;
     }

     if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
         perror("setsockopt");
         return -1;
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
     return -1;
  }

  if (listen(*sockfd, BACKLOG) == -1) {
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

// get sockadr, IPv4 or IPv6:
void *get_client_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
     return &(((struct sockaddr_in*) sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

int get_serv_addr(struct addrinfo *hints, struct addrinfo *servinfo) {
  int rv;

  memset(hints, 0, sizeof(*hints));
  hints->ai_family = AF_UNSPEC;
  hints->ai_socktype = SOCK_STREAM;
  hints->ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL,  PORT, hints, &servinfo)) != 0) {
     fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
     return 1;
  }
  return 0;
}
