
/* socket.c */
int create_socket(void);
int begin_listening(int);
int accept_connection(int);

/* request.c */
#define HEADER_LEN 4096
#define CONTENT_LEN 4096 

typedef struct req {
   char *method;
   char *url;
   char *content;
} req;

int serve_request(int);
int read_request(int, struct req *);
int parse_headers(struct req *);
int write_response(int, struct req *);
int send_file(int, int);
