#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include "headerfile.h"

thread_pool *build_thread_pool(void) {
   thread_pool *t_pool;
   int i;

   //allocate memory for threadpool, threads, connections separate
   t_pool = malloc(sizeof(t_pool));
   t_pool->threads = malloc(sizeof(pthread_t) * MAX_THREADS);

   pthread_mutex_init(&(t_pool->thread_lock), NULL); //initialize mutex to create exclusive access
   pthread_cond_init(&(t_pool->signal), NULL); // initialize conditional variable

   for (i = 0; i < MAX_THREADS; i++) {
      // create new thread and place it in a wait state
      if(pthread_create(&(t_pool->threads[i]), NULL, (void *) init_worker_thread, t_pool) != 0) {
         //pthread does not set errno, so use return value of pthread_join
         printf("Error Creating Thread: %i", pthread_join(t_pool->threads[i], NULL));
         return NULL;
      }
   }
   return t_pool;
}

void init_worker_thread(thread_pool *t_pool) {
  int job; 
  job_queue *job_q;
  job_q = malloc(sizeof(job_queue));
  job_q->tid = pthread_self();
  job_q->head = 0;
  job_q->tail = 0;
  job_q->count = 0;

  while(1) {

    // TODO: poll(2) if there is a request to be satisfied
    while(job_q->count == 0) {
       printf("Thread %p in Wait State.\n", pthread_self());
       pthread_cond_wait(&(t_pool->signal), &(t_pool->thread_lock)); // wait until conditional variable (signal) changes
    }
  
    job = job_q->jobs[job_q->head++]; // get next job from queue

    if (job_q->head == MAX_JOBS) {
       job_q->head = 0;
    }

    job_q->count -= 1;
    // perform job: serve request (connection has already been accepted)
    serve_request(job);
  }
  free(job_q);
  pthread_exit(NULL);
}

void cleanup_tpool(thread_pool *tpool) {
   int i;
   for (i = 0; i < MAX_THREADS; i++) {
      pthread_join(tpool->threads[i], NULL);
   }
   free(tpool);
}
