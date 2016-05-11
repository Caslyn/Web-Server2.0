#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

int status;
struct addrinfo hints; // points to struct addrinfo already filled with info
struct addrinfo *servinfo; // pointer to results of getaddrinfo()

memset(&hints, 0, sizeof(hints)); // writes 0s into hints
hints.ai_family = AF_UNSPEC; // don't care if IPv4 or IPv6
hints.ai_socktype = SOCK_STREAM; //TCP stream sockets
hints.ai_flags = AI_PASSIVE; // fill in my IP for me

if((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
  fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
  exit(1);
}

