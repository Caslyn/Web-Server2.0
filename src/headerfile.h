/* client.c */
int get_address(void);
int connect_client(char *, int *);
int client_recv(int *);

/* server.c */
int get_address(void);
int connect_server(int *);
int accept_connection(int *);
