#include <netdb.h>
#include <netinet/in.h>
#include "listener.c"

/* server.c */
int listen_on_socket(int *);
int accept_connection(int *);

/* listener.c */
void *get_client_addr(struct sockaddr *);
int get_serv_addr(struct addrinfo *, struct addrinfo *);
