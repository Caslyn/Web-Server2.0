#include "headerfile.h"

int main() {
  int sockfd;

  if(connect_server(&sockfd) == -1) {
     return 1;
  }

  if(accept_connection(&sockfd) == -1) {
     return 1;
  }
  return 0;
}
