#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>  
#include <stdbool.h>
#include <semaphore.h>

#define BUFFER_SIZE 5

typedef int buffer_item;
buffer_item buffer[BUFFER_SIZE];

typedef struct {
	int tid_param;
	pthread_mutex_t *mutex_param;
	sem_t *empty_param;
	sem_t *full_param;
} sem_params;

int prod_index = 0;
int cons_index = 0;

void insert_item(buffer_item prod, int index) {
	buffer[index] = prod;
}

buffer_item remove_item(int index) {
	buffer_item item_removed = buffer[index];
	buffer[index] = -1;
	return item_removed;
}

void *producer(void *param) {
	int tid = ((sem_params *)param)->tid_param;
	pthread_mutex_t *mutex = ((sem_params *)param)->mutex_param;
	sem_t *empty = ((sem_params *)param)->empty_param;
	sem_t *full = ((sem_params *)param)->full_param;
	
	int index;
	buffer_item item;
	
	/* display name of created thread (used for testing) */
	//printf("Producer %d created\n", tid);
	
	while (true) {
		/* sleep for a random period of time: 0-4 seconds */
		sleep(rand()%5);
		
		/* generate a random number */
		item = rand();
		
		/* acquire mutex lock and semaphore */
		sem_wait(empty);
		pthread_mutex_lock(mutex);
		
		/* insert an item */
		index = prod_index % BUFFER_SIZE;
		insert_item(item, index);		
		prod_index++;
		
		/* print producer action */
		printf("Producer %d inserted item %d into buffer[%d]\n", tid, item, index);
		
		/* display buffer (used for testing) */
		/*
		for (int i=0; i<BUFFER_SIZE; i++) {
			printf("%d %d\n", i, buffer[i]);
		}
		*/
		
		/* release mutex lock and semaphore */
		pthread_mutex_unlock(mutex);
		sem_post(full);
	}
}

void *consumer(void *param) {
	int tid = ((sem_params *)param)->tid_param;
	pthread_mutex_t *mutex = ((sem_params *)param)->mutex_param;
	sem_t *empty = ((sem_params *)param)->empty_param;
	sem_t *full = ((sem_params *)param)->full_param;
	
	int index;
	buffer_item removed_item;
	
	/* display name of created thread (used for testing) */
	//printf("Consumer %d created\n", tid);
	
	while (true) {
		/* sleep for a random period of time: 0-4 seconds */
		sleep(rand()%5);
		
		/* acquire mutex lock and semaphore */
		sem_wait(full);
		pthread_mutex_lock(mutex);
		
		/* remove an item */
		index = cons_index % BUFFER_SIZE;
		removed_item = remove_item(index);
		cons_index++;
		
		/* print consumer action */
		printf("Consumer %d consumed item %d from buffer[%d]\n", tid, removed_item, index);
		
		/* display buffer (used for testing) */
		/*
		for (int i=0; i<BUFFER_SIZE; i++) {
			printf("%d %d\n", i, buffer[i]);
		}
		*/
		
		/* release mutex lock and semaphore */
		pthread_mutex_unlock(mutex);
		sem_post(empty);
	}
}

int main(int argc, char *argv[]) {
	/* Initializes variables */
	time_t t;
	int sleep_time;
	int n_prod;
	int n_cons;
	
	/* Initializes random number generator */
	srand((unsigned) time(&t));
	
	/* 1. Get command-line arguments argv[1],argv[2],argv[3] */
	sleep_time = atoi(argv[1]);
	n_prod = atoi(argv[2]);
	n_cons = atoi(argv[3]);
	
	/* 2. Initialize semaphores and mutex lock */
	pthread_mutex_t mutex;
	sem_t empty;
	sem_t full;
	
	pthread_mutex_init(&mutex, NULL);
	sem_init(&empty, 0, BUFFER_SIZE);
	sem_init(&full, 0, 0);
	
	/* 3. Initialize buffer */
	for (int i=0; i<BUFFER_SIZE; ++i) {
		buffer[i] = -1;
	}
	
	/* 4. Create producer thread(s) */
	pthread_t ptid[n_prod];
	
	// the reason these data structures have to be redefined each loop is because otherwise there is a chance the tids will be wrong
	// the reason the tids might be wrong appears to be because of modification of the same variables by lines of codes executing almost concurrently
	
	for (int i_p=0; i_p < n_prod; ++i_p) {
		sem_params *data_p = (sem_params *) malloc(sizeof(sem_params));
		data_p->mutex_param = &mutex;
		data_p->empty_param = &empty;
		data_p->full_param = &full;
		data_p->tid_param = i_p;
		pthread_create(&ptid[i_p], NULL, producer, data_p);
	}	
	
	/* 5. Create consumer thread(s) */
	pthread_t ctid[n_cons];
	
	for (int i_c=0; i_c < n_cons; ++i_c) {
		sem_params *data_c = (sem_params *) malloc(sizeof(sem_params));
		data_c->mutex_param = &mutex;
		data_c->empty_param = &empty;
		data_c->full_param = &full;
		data_c->tid_param = i_c;
		pthread_create(&ctid[i_c], NULL, consumer, data_c);
	}
	
	/* 6. Sleep and thereafter terminate the C program */
	sleep(sleep_time);
	return 0;
	
	
}