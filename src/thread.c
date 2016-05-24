#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include "headerfile.h"

thread_pool *build_t_pool(void) {
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
  job_queue *j_queue = malloc(sizeof(job_queue));
  j_queue->head = 0;
  j_queue->tail = 0;
  j_queue->count = 0;
  
  // TODO: poll(2) if there is a request to be satisfied
  while(1) {
    
    
  }
}

