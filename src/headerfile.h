
/* socket.c */
int create_socket(void);
int begin_listening(int);
int accept_connection(int);

/* request.c */
#define HEADER_LEN 4096
#define CONTENT_LEN 4096

typedef struct req {
   char *url;
   char *content;
} req;

int serve_request(int);
int read_request(int, req *);
int parse_headers(req *);
int write_response(int, req *);
int send_file(int, int);

/* thread.c */
#define MAX_THREADS 10
#define MAX_CONNECTIONS 100

// custom attribute for each thread
typedef struct job_queue {
 int head; // pointer to beginning of queue
 int tail; // pointer to the next available place in queue
 int count; // number of jobs
} job_queue;

typedef struct thread_pool {
  pthread_mutex_t thread_lock; // lock so one thread can have exclusive access
  pthread_cond_t signal; // conditional signal to lock/unlock
  pthread_t *threads; // threads in thread pool
} thread_pool

thread_pool *build_thread_pool(void);
void init_worker_thread(thread_pool *thread_pool);

/* poll.c */
#define TRUE 1
#define FALSE 0
void poll_wait(int, int);
