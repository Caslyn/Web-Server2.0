#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "headerfile.h"

void poll_wait(int sockfd, int events) {
   int n;
   struct pollfd pollfds[MAX_JOBS];
   memset((char *) &pollfds, 0, sizeof(pollfds));
   printf("Executing Poll\n\n");
   pollfds[0].fd = sockfd;
   pollfds[0].events = events;
   // passing in a timeout of -1 to block indefinitely

   if ((n = poll(pollfds, 1, -1)) < 0) {
      perror("poll");
   };
}
