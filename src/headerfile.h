/* request.c */
#define HEADER_LEN 1024
#define CONTENT_LEN 1024

typedef struct req {
   char *url;
   char *content;
} req;

int serve_request(int);
int read_request(int, req *);
int parse_headers(req *);
int write_response(int, req *);
int send_file(int, int, off_t, char*);
void send_response_headers(int, off_t, char *, char *);
char *get_file_type(char *);

/* thread.c */
#include <pthread.h>
#define MAX_THREADS 5
#define MAX_CONNECTIONS 25
#define MAX_JOBS 5

typedef struct job_queue {
  pthread_t tid; // thread id each queue belongs to
  int *jobs;
  int head;
  int tail;
  int count;
  pthread_mutex_t thread_lock;
  pthread_cond_t signal;
} job_queue;

typedef struct job_queue_pool {
 job_queue *job_queues;
 int head;
 int tail;
 int count;
} job_queue_pool;
 
typedef struct thread_pool {
  pthread_t *threads; // threads in thread pool
} thread_pool;

thread_pool *build_thread_pool(job_queue_pool *);
job_queue_pool *build_jobq_pool(void);

void goto_sleep(job_queue *);
void wake_up(job_queue *);
void clean_up_pools(thread_pool*, job_queue_pool *);

/* poll.c */
#define TRUE 1
#define FALSE 0
void poll_wait(int, int);

/* socket.c */
#define BACKLOG 10
#define PORT "5000"
int create_socket(void);
int begin_listening(int);
int distr_connections(int, thread_pool *, job_queue_pool *);
int accept_connection(int);
 
