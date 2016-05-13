#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "headerfile.h"

#define PORT "5000" // the port clients will be connecting to

static struct sockaddr_storage client_addr; // clients address information
static socklen_t sin_size;

int accept_connection(int *sockfd) {
  int new_fd;
  char s[INET6_ADDRSTRLEN];

  while(1) {
     sin_size = sizeof(client_addr);
     new_fd = accept(*sockfd, (struct sockaddr *) &client_addr, &sin_size);

     if (new_fd == -1) {
        perror("accept");
        return -1;
     }

     inet_ntop(client_addr.ss_family, get_client_addr((struct sockaddr *) &client_addr), s, sizeof(s));
     printf("server: got connection from %s\n", s);

     if (!fork()) { // this is the child process
       close(*sockfd); // child doesn't need the listening socket
       if (send(new_fd, "Hello, world!", 13, 0) == -1) {
          perror("send");
          close(new_fd);
          return -1;
       }
       exit(0);
     }
     close(new_fd); // parent doesn't need the new fd
  }
  return 0;
}
