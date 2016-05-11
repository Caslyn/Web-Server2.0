#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

void prepare_socket(void) {
  int status;
  struct addrinfo hints; // points to struct addrinfo already filled with info
  struct addrinfo *servinfo; // pointer to results of getaddrinfo()

  memset(&hints, 0, sizeof(hints)); // writes 0s into hints
  hints.ai_family = AF_UNSPEC; // don't care if IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; //TCP stream sockets
  hints.ai_flags = AI_PASSIVE; // assign address of my local host to the socket structs

  // servinfo points to (linkedlist) results from getaddrinfo
  if((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    exit(1);
  }

  freeaddrinfo(servinfo); //free linked-list
}
