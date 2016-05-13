#include <netdb.h>
#include <netinet/in.h>
#include "listener.c"

/* server.c */

/* socket.c */
int create_socket(int *);
int listen_on_socket(int *);
int accept_connection(int *);

/* listener.c */
void *get_client_addr(struct sockaddr *);
