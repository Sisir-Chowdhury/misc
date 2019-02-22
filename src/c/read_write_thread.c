#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS  1
#define TCOUNT 20
#define COUNT_LIMIT 0

int     pkt_len = 0;
int     thread_ids[1] = {0};
pthread_mutex_t pkt_len_mutex;
pthread_cond_t pkt_len_threshold_cv;

void *read_pkt(void *t) 
{
   long my_id = (long)t;
   int NUM_BYTE = 125;

   //printf("Starting read_pkt(): thread %ld\n", my_id);

   /*
   Lock mutex and wait for signal.  Note that the pthread_cond_wait 
   routine will automatically and atomically unlock mutex while it waits. 
   Also, note that if COUNT_LIMIT is reached before this routine is run by
   the waiting thread, the loop will be skipped to prevent pthread_cond_wait
   from never returning. 
   */
   while(1) {
      pthread_mutex_lock(&pkt_len_mutex);
      //printf ("**** reader thread COUNT: %d\n", pkt_len);
      while (pkt_len==COUNT_LIMIT) {
        pthread_cond_wait(&pkt_len_threshold_cv, &pkt_len_mutex);
        printf("read_pkt(): thread %ld Condition signal received.\n", my_id);
      }
      pkt_len -= NUM_BYTE;
      printf("read_pkt(): thread %ld pkt_len now = %d. Read %d byte Data \n", my_id, pkt_len, NUM_BYTE);
      pthread_cond_signal(&pkt_len_threshold_cv);
      pthread_mutex_unlock(&pkt_len_mutex);
      //printf("read_pkt(): thread %ld unlocked Mutex \n", my_id);
   }
   pthread_exit(NULL);
}

void *mirror_pkt(void *t) 
{
   long my_id = (long)t;

   //printf("Starting mirror_pkt(): Main thread %ld\n", my_id);

   /*
   Lock mutex and wait for signal.  Note that the pthread_cond_wait 
   routine will automatically and atomically unlock mutex while it waits. 
   Also, note that if COUNT_LIMIT is reached before this routine is run by
   the waiting thread, the loop will be skipped to prevent pthread_cond_wait
   from never returning. 
   */
   while(1) {
      pthread_mutex_lock(&pkt_len_mutex);
      //printf ("**** main thread COUNT: %d\n", pkt_len);
      while (pkt_len>COUNT_LIMIT) {
        pthread_cond_wait(&pkt_len_threshold_cv, &pkt_len_mutex);
        printf("mirror_pkt(): thread %ld Condition signal received.\n", my_id);
      }
      pkt_len += 125;
      printf("mirror_pkt(): thread %ld pkt_len now = %d. Written %d byte Data \n", my_id, pkt_len, pkt_len);
      pthread_cond_signal(&pkt_len_threshold_cv);
      pthread_mutex_unlock(&pkt_len_mutex);
      //printf("mirror_pkt(): thread %ld unlocked Mutex \n", my_id);
   }
   pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
   int i, rc;
   long t1=0, t2=1, t3=2;
   pthread_t threads[3];
   pthread_attr_t attr;

   /* Initialize mutex and condition variable objects */
   pthread_mutex_init(&pkt_len_mutex, NULL);
   pthread_cond_init (&pkt_len_threshold_cv, NULL);

   /* For portability, explicitly create threads in a joinable state */
   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
   pthread_create(&threads[1], &attr, read_pkt, (void *)t2);
   mirror_pkt((void *)t1);

   /* Wait for all threads to complete */
   for (i=0; i<NUM_THREADS; i++) {
     pthread_join(threads[i], NULL);
   }
   printf ("Main(): Waited on %d  threads. Done.\n", NUM_THREADS);

   /* Clean up and exit */
   pthread_attr_destroy(&attr);
   pthread_mutex_destroy(&pkt_len_mutex);
   pthread_cond_destroy(&pkt_len_threshold_cv);
   pthread_exit(NULL);
} 
