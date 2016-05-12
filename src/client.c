/*
** client.c -- a stream socket for the client
*/

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "headerfile.h"

#define PORT "3490" // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes client can receive at once

static struct addrinfo hints, *servinfo, *p;

/* initiate client connection
  * expect client to provide hostname to connect to
 */

int main(int argc, char *argv[]) {
  int rv, client_sockfd;
  char s[INET6_ADDRSTRLEN];

  if (argc != 2) {
    fprintf(stderr, "usage: client hostname\n");
    return 1;
  }

  if ((rv = get_address()) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  if (connect_client(s, &client_sockfd) != 0) {
    fprintf(stderr, "client: failed to connect\n");
    return 1;
  } else {
    printf("client: connecting to %s\n", s);
  }
  client_recv(&client_sockfd);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
  if(sa->sa_family == AF_INET) {
     return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int get_address(void) {
  memset(&hints, 0, sizeof(hints)); // writes 0s into hints
  hints.ai_family = AF_UNSPEC; // don't care if IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; //TCP stream sockets

  // servinfo points to (linkedlist) results from getaddrinfo
  return getaddrinfo(NULL, "3490", &hints, &servinfo);
}

int connect_client(char *s, int *client_sockfd) {
  // loop through all the results and connect to the first one we can
  for (p = servinfo; p != NULL; p = p->ai_next){
    if ((*client_sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(*client_sockfd, p->ai_addr, p->ai_addrlen) == -1) {
       close(*client_sockfd);
       perror("client: connect");
       continue;
    }
    break;
  }

  if (p == NULL) {
    return 1;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
  freeaddrinfo(servinfo); // all done with this structure
  return 0;
}

int client_recv(int *client_sockfd) {
  int numbytes;
  char buf[MAXDATASIZE];

  if ((numbytes = recv(*client_sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
    perror("client recv");
    exit(1);
  }

  buf[numbytes] = '\0';
  printf("client: recieved %s\n", buf);
  close(*client_sockfd);
  return 0;
}
