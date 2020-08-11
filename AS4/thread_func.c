/*Dewansh Singh 170241*/
#include<pthread.h>
#include "common.h"
 
/*TODO:  
     Insert approproate synchronization statements
     to make it work with multiple threads
*/
pthread_mutex_t lock;

void *hashit(void *arg)
/*Argument is the end pointer*/
{
   char *cptr;
   unsigned long *chash;
   char *endptr = (char *)arg;   // END pointer

   while(1){
        pthread_mutex_lock(&lock);
        if(dataptr >= endptr){
              pthread_mutex_unlock(&lock);
              break;
        }
        cptr = dataptr;
        dataptr += BLOCK_SIZE;
        chash = optr;
        optr++;
        pthread_mutex_unlock(&lock);
        /*   Perform the real calculation. The following line should not be inside any locks*/
        *chash = calculate_and_store_hash(cptr, endptr); 
  }
  pthread_exit(NULL); 
}