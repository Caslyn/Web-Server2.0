#include "headerfile.h"

int main() {
  int sockfd;

  if(create_socket(&sockfd) == -1) {
     return 1;
  }
  
  if(listen_on_socket(&sockfd) == -1) {
     return 1;
  }

  if(accept_connection(&sockfd) == -1) {
     return 1;
  }

 return 0;
}
