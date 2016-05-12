#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include "headerfile.h"

/* start web server 
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
