#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include "headerfile.h"

thread_pool *build_thread_pool(job_queue_pool *jobq_pool) {
   thread_pool *t_pool;
   int i;

   //allocate memory for threadpool, threads, connections separate
   t_pool = malloc(sizeof(t_pool));
   t_pool->threads = malloc(sizeof(pthread_t) * MAX_THREADS);


   for (i = 0; i < MAX_THREADS; i++) {
      thread_info *t_info = malloc(sizeof(thread_info));
       t_info->t_pool = t_pool;
      // assign job queue to thread
      t_info->job_q = &jobq_pool->job_queues[i];
      // create new thread and place it in a wait state
      if(pthread_create(&(t_pool->threads[i]), NULL, (void *) init_worker_thread, t_info) != 0) {
         //pthread does not set errno, so use return value of pthread_join
         printf("Error Creating Thread: %i", pthread_join(t_pool->threads[i], NULL));
         return NULL;
      }
      free(t_info);
   }
   return t_pool;
}

void init_worker_thread(thread_info *t_info) {
    int job; 
    t_info->thread_id = pthread_self();
    job_queue *job_q = t_info->job_q;

    printf("Thread %p Waiting for Work.\n", pthread_self());
    // wait for job that falls into jobx queue (ie a connnection that has been accepted but waiting to be served
    job = job_q->jobs[job_q->head++]; // get next job from personal job queue
    poll_wait(job, POLLWRNORM);
  
    if (job_q->head == MAX_JOBS) { // if job queue is filled, start from beginning of queue next time around
       job_q->head = 0;
    }

    job_q->count -= 1;
    // perform job: serve request (connection has already been accepted)
    serve_request(job);
    printf("Terminating thread...\n");
    pthread_exit(NULL); // terminate thread
    printf("Freeing thread's job queue...\n");
    free(job_q);
}
 
job_queue_pool *build_jobq_pool() {
   int i;
   job_queue *job_q;
   // allocate space for job queues for all threads
   job_queue_pool *job_q_pool;
   job_q_pool = (job_queue_pool *) malloc(sizeof(job_queue_pool)); 
   job_q_pool->job_queues = malloc(sizeof(job_queue) * MAX_THREADS);
   job_q_pool->head = 0;
   job_q_pool->tail = 0;
   job_q_pool->count = 0;
   // allocate storage for all job queues for all threads
   for(i = 0; i < MAX_THREADS; i++) {
      job_q =(job_queue *) malloc(sizeof(job_queue));
      job_q->head = 0;
      job_q->tail = 0;
      job_q->count = 0;
      job_q->jobs = malloc(MAX_JOBS);
      job_q_pool->job_queues[i] = *job_q;
   }
   return job_q_pool;
}

void cleanup_tpool(thread_pool *tpool) {
   int i;
   for (i = 0; i < MAX_THREADS; i++) {
      pthread_join(tpool->threads[i], NULL);
   }
   free(tpool);
}
