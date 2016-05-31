#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "headerfile.h"

static void *get_client_addr(struct sockaddr *);
static struct addrinfo hints, *servinfo, *p;
static struct sockaddr_storage client_addr; // clients address information
static socklen_t sin_size;

// get sockadr, IPv4 or IPv6:
static void *get_client_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
     return &(((struct sockaddr_in*) sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

int create_socket(void){
  int sockfd;
  int yes = 1, rv;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL,  PORT, &hints, &servinfo)) != 0) {
     fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
     exit(0);
  }

  // loop through all the results and bind the first we can
  for (p = servinfo; p != NULL; p = p->ai_next) {
     if((sockfd  = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        perror("server: socket");
        continue;
     }

     if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
       close(sockfd);
       perror("setsockopt");
         continue;
     }

     if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
        perror("server:bind");
        continue;
     }
     break;
  }
  freeaddrinfo(servinfo); // all done with this structure

  if (p == NULL){
     fprintf(stderr, "server:failed to bind\n");
     exit(1);
     return -1;
  }
  return sockfd;
}

int begin_listening(int sockfd){

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    return -1;
  }

  printf("server: waiting for connection...\n");
  return 0;
}
// distribute connections to job queues
int distr_connections (int sockfd, thread_pool *t_pool, job_queue_pool *job_q_pool){
  int new_fd, next_job_q, next_job;
  job_queue *chosen_job_q;

  while(1) {
    sin_size = sizeof(client_addr);
    poll_wait(sockfd, POLLIN); // wait for connection
    printf("A connection is ready to accept\n");

    if((new_fd = accept_connection(sockfd)) < 0) {
      continue;
     }
    next_job_q = job_q_pool->tail + 1; // next available job queue

    if (next_job_q == MAX_THREADS) { // If next is the beyond the number of threads(1:1 with job queues), then assign next jobqueue to first
      next_job_q = 0;
    }
    job_q_pool->tail = next_job_q;
   // add connection to the next available job queue
   chosen_job_q = &job_q_pool->job_queues[job_q_pool->tail]; 
   // next available job spot in chosen job queue
   next_job = chosen_job_q->tail + 1;

   if (next_job == MAX_JOBS) {
      next_job = 0;
   }
   // add new_sockfd to the end of the chosen job queue
   chosen_job_q->jobs[chosen_job_q->tail] = new_fd;
   chosen_job_q->count++;
   EV_SET(&chosen_job_q->ke, chosen_job_q->jobs[chosen_job_q->tail], EVFILT_READ, EV_ADD, 0, 0, NULL); 
   if (kevent(chosen_job_q->kq, &chosen_job_q->ke, 1, NULL, 0,  NULL) < 0) {
      perror("signal kevent");
   }
   chosen_job_q->tail = next_job;
  }

  clean_up_pools(t_pool, job_q_pool);
  return 0;
}

int accept_connection(int sockfd) {
  int new_fd, pid;
  char s[INET6_ADDRSTRLEN];

   if((new_fd = accept(sockfd, (struct sockaddr *) &client_addr, &sin_size)) < 0) {
      perror("accept");
      return -1;
   }

   inet_ntop(client_addr.ss_family, get_client_addr((struct sockaddr *) &client_addr), s, sizeof(s));
   printf("server: accepted connection from %s\n", s);

  return new_fd;
}
