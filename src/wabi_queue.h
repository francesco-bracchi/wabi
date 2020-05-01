#ifndef wabi_queue_h

#define wabi_queue_h

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "wabi_value.h"
#include "wabi_vm.h"

typedef struct wabi_queue_elem
{
  wabi_vm vm;
  struct wabi_queue_elem *next;
} wabi_queue_elem_t;

typedef wabi_queue_elem_t* wabi_queue_elem;

typedef struct wabi_queue_struct
{
  pthread_mutex_t lock;
  pthread_cond_t cond;

  wabi_word size;
  wabi_queue_elem head;
  wabi_queue_elem tail;
} wabi_queue_t;

typedef wabi_queue_t* wabi_queue;


static inline void
wabi_queue_init(wabi_queue queue)
{
  queue->head = NULL;
  queue->tail = NULL;
  queue->size = 0;
  pthread_mutex_init(&queue->lock, NULL);
  pthread_cond_init(&queue->cond, NULL);
}


static inline void
wabi_queue_destroy(wabi_queue queue)
{
  wabi_queue_elem elem;

  while(queue->head) {
    elem = queue->head;
    queue->head = queue->head->next;
    wabi_vm_destroy(elem->vm);
    free(elem->vm);
    free(elem);
  }
  pthread_mutex_destroy(&queue->lock);
  pthread_cond_destroy(&queue->cond);
}


static inline void
wabi_queue_enq(wabi_queue queue, wabi_vm vm)
{
  wabi_queue_elem elem;
  pthread_mutex_lock(&queue->lock);
  if(queue->size) {
    queue->size++;
    elem = queue->tail;
    queue->tail = (wabi_queue_elem) malloc(sizeof(wabi_queue_elem));
    elem->next = queue->tail;
    queue->tail->vm = vm;
  } else  {
    queue->head = queue->tail = (wabi_queue_elem) malloc(sizeof(wabi_queue_elem));
    queue->head->vm = vm;
    queue->head->next = NULL;
    queue->size = 1;
  }
  pthread_cond_signal(&queue->cond);
  pthread_mutex_unlock(&queue->lock);
}


static inline wabi_vm
wabi_queue_deq(wabi_queue queue)
{
  wabi_vm res;
  wabi_queue_elem elem;
  pthread_mutex_lock(&queue->lock);
  while(!queue->size) {
    pthread_cond_wait(&queue->cond, &queue->lock);
  }
  queue->size--;
  elem = queue->head;
  queue->head = elem->next;
  res = elem->vm;
  free(elem);
  pthread_mutex_unlock(&queue->lock);
  return res;
}

#endif
