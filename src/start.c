#include "headerfile.h"

int main() {
  int sockfd;
  thread_pool *tpool;
  job_queue_pool *jobq_pool;

  if((sockfd = create_socket()) == -1) {
     return 1;
  }

  if(begin_listening(sockfd) == -1) {
     return 1;
  }

  jobq_pool = build_jobq_pool();
  tpool = build_thread_pool(jobq_pool);

  distr_connections(sockfd, tpool, jobq_pool);

 return 0;
}
