#include "headerfile.h"

int main() {
  int sockfd;

  if((sockfd = create_socket()) == -1) {
     return 1;
  }
  
  if(begin_listening(sockfd) == -1) {
     return 1;
  }

  if(accept_connection(sockfd) == -1) {
     return 1;
  }

 return 0;
}
