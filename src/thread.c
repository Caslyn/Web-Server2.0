#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/event.h>
#include <sys/poll.h>
#include "headerfile.h"

thread_pool *build_thread_pool(job_queue_pool *jobq_pool) {
   thread_pool *t_pool;
   job_queue *job_q;
   int i;

   t_pool = malloc(sizeof(t_pool));
   t_pool->threads = malloc(sizeof(pthread_t) * MAX_THREADS);

   for (i = 0; i < MAX_THREADS; i++) {
      job_q = &jobq_pool->job_queues[i];
      if(pthread_create(&(t_pool->threads[i]), NULL, (void *) goto_sleep, job_q) != 0) {
         //pthread does not set errno, so use return value of pthread_join
         printf("Error Creating Thread: %i", pthread_join(t_pool->threads[i], NULL));
         return NULL;
      }
   }
   return t_pool;
}

void goto_sleep(job_queue *job_q) {
  job_q->kq = kqueue();
  kevent(job_q->kq, &job_q->ke, 1, NULL, 0,  NULL);
    while(job_q->count == 0) {
      kevent(job_q->kq, NULL, 0, &job_q->ke, 1, NULL); // this eventually blocks other reqs
      memset(&job_q->ke, 0, sizeof(struct kevent));
    }
  wake_up(job_q);
}

void wake_up(job_queue *job_q) {
    int job = job_q->jobs[job_q->head++]; // get next job from job queue
    printf("Waking up Thread %p\n", pthread_self());
    if (job_q->head == MAX_JOBS) {
       job_q->head = 0;
    }

    job_q->count -= 1;
    serve_request(job);
    goto_sleep(job_q);
}
 
job_queue_pool *build_jobq_pool() {
   job_queue *job_q;
   job_queue_pool *job_q_pool;
   int i;

   job_q_pool = (job_queue_pool *) calloc(1, sizeof(job_queue_pool)); 
   job_q_pool->job_queues = (job_queue *) malloc(sizeof(job_queue) * MAX_THREADS);
   
   // allocate storage for all job queues for all threads
   for(i = 0; i < MAX_THREADS; i++) {
      job_q = (job_queue *) calloc(1, sizeof(job_queue));
      job_q->jobs = (int *) malloc(MAX_JOBS);
      job_q_pool->job_queues[i] = *job_q;
   }
   return job_q_pool;
}

void clean_up_pools(thread_pool *t_pool, job_queue_pool *job_q_pool) { 
  job_queue *job_q;
  int i;
   for (i = 0; i < MAX_THREADS; i++) {
      pthread_join(t_pool->threads[i], NULL);
      free(job_q_pool->job_queues[i].jobs);
      free(&job_q_pool->job_queues[i]);
   }

   free(job_q_pool->job_queues);
   free(job_q_pool);  
   free(t_pool);
}
