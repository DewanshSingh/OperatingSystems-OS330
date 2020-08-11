/*Dewansh Singh
170241*/

#include "common.h"
/*Function templates. TODO*/
static int atomic_add(unsigned *ptr, long val)
{
       int ret = 0;
       asm volatile( 
                     "lock add %%rsi, (%%rdi);"
                     "pushf;"
                     "pop %%rax;" 
                     "movl %%eax, %0;"
                     : "=r" (ret)
                     : 
                     : "memory", "rax"
                    );
     if(ret & 0x80)
               ret = -1;
     else if(ret & 0x40)
               ret = 0;
     else
               ret = 1;
     return ret;
}

void done_one(struct input_manager *in, int tnum)
{
   pthread_mutex_lock(&(in->lock));
   in->being_processed[tnum-1] = NULL; 
   pthread_cond_broadcast(&(in->cond));
   pthread_mutex_unlock(&(in->lock));
 
}

int read_op(struct input_manager *in, op_t *op, int tnum)
{
  unsigned size = sizeof(op_t);
  pthread_mutex_lock(&(in->lock));
    if(in->curr > in->data + in->size)
    {
      pthread_mutex_unlock(&(in->lock));
      return -1;    // Failed
    }
  memcpy(op, in->curr, size - sizeof(unsigned long));  //Copy till data ptr     
   if(op->op_type == GET || op->op_type == DEL){
       in->curr += size - sizeof(op->datalen) - sizeof(op->data);
   }else if(op->op_type == PUT){
       in->curr += size - sizeof(op->data);
       op->data = in->curr;
       in->curr += op->datalen;
   }else{
       assert(0);
   }
   if(in->curr > in->data + in->size)
        {
          pthread_mutex_unlock(&(in->lock));
          return -1;    // Failed
        }
        
   in->being_processed[tnum-1] = op;

   int itrm=1;
   while(itrm)
   {
    itrm =0;
    for(int i =0 ;i<THREADS;i++)
      {
        if(i+1==tnum) 
          continue;
        if( (in->being_processed[i]!=NULL) && ( in->being_processed[i]->id == in->being_processed[tnum-1]->id) && (in->being_processed[i]->key == in->being_processed[tnum-1]->key) )
          itrm =1;
      }
    if(itrm)
    pthread_cond_wait(&(in->cond), &(in->lock));

   }

   pthread_mutex_unlock(&(in->lock));
   return 0; 
 
}

int lookup(hash_t *h, op_t *op)
{
  unsigned ctr;
  unsigned hashval = hashfunc(op->key, h->table_size);
  hash_entry_t *entry = h->table + hashval;
  ctr = hashval;
  pthread_mutex_lock(&(entry->lock));
  while((entry->key || entry->id == (unsigned) -1) && 
         entry->key != op->key && ctr != hashval - 1){
      
      ctr = (ctr + 1) % h->table_size;
      pthread_mutex_unlock(&(entry->lock));
      entry = h->table + ctr; 
      pthread_mutex_lock(&(entry->lock));

  } 
 if(entry->key == op->key){
      op->datalen = entry->datalen;
      op->data = entry->data;
      return 0;
 }
 pthread_mutex_unlock(&(entry->lock));
 return -1;    // Failed
}

int insert_update(hash_t *h, op_t *op)
{
   unsigned ctr;
   unsigned hashval = hashfunc(op->key, h->table_size);
   hash_entry_t *entry = h->table + hashval;
   
   assert(h && h->used < h->table_size);
   assert(op && op->key);

   ctr = hashval;
   pthread_mutex_lock(&(entry->lock));
   while((entry->key || entry->id == (unsigned) -1) &&
   entry->key != op->key && ctr != hashval - 1){
         ctr = (ctr + 1) % h->table_size;
         pthread_mutex_unlock(&(entry->lock));
         entry = h->table + ctr; 
         pthread_mutex_lock(&(entry->lock)); 
   } 

  assert(ctr != hashval - 1);

  if(entry->key == op->key){  //It is an update
      entry->id = op->id;
      entry->datalen = op->datalen;
      entry->data = op->data;
      pthread_mutex_unlock(&(entry->lock));
      return 0;
  }
  assert(!entry->key);   // Fresh insertion
 
  entry->id = op->id;
  entry->datalen = op->datalen;
  entry->key = op->key;
  entry->data = op->data;
  pthread_mutex_unlock(&(entry->lock));
  atomic_add(&(h->used),0x1);
  return 0; 
  
}

int purge_key(hash_t *h, op_t *op)
{
   unsigned ctr;
   unsigned hashval = hashfunc(op->key, h->table_size);
   hash_entry_t *entry = h->table + hashval;
   
   ctr = hashval;
   pthread_mutex_lock(&(entry->lock));
   while((entry->key || entry->id == (unsigned) -1) && 
          entry->key != op->key && ctr != hashval - 1){

         ctr = (ctr + 1) % h->table_size;
       pthread_mutex_unlock(&(entry->lock));
         entry = h->table + ctr; 
         pthread_mutex_lock(&(entry->lock));

   } 

   if(entry->key == op->key){  //Found. purge it
      entry->id = (unsigned) -1;  //Empty but deleted
      entry->key = 0;
      entry->datalen = 0;
      entry->data = NULL;
      pthread_mutex_unlock(&(entry->lock));
      atomic_add(&(h->used),-1);
      return 0;
   }
  pthread_mutex_unlock(&(entry->lock));  
  return -1;    // Bogus purge
}
