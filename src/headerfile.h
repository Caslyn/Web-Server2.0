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
} job_queue;

typedef struct job_queue_pool {
 job_queue *job_queues;
 int head;
 int tail;
 int count;
} job_queue_pool;
 
typedef struct thread_pool {
  pthread_mutex_t thread_lock; // lock so one thread can have exclusive access
  pthread_cond_t signal; // conditional signal to lock/unlock
  pthread_t *threads; // threads in thread pool
} thread_pool;

typedef struct thread_info {
  pthread_t thread_id;
  job_queue *job_q;
  thread_pool *t_pool;
} thread_info;

thread_pool *build_thread_pool(job_queue_pool *jobq_pool);
void init_worker_thread(thread_info *t_info);
job_queue_pool *build_jobq_pool(void);

/* poll.c */
#define TRUE 1
#define FALSE 0
void poll_wait(int, int);

/* socket.c */
int create_socket(void);
int begin_listening(int);
int distr_connections(int, thread_pool *, job_queue_pool *);
int accept_connection(int);
 
