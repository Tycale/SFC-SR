#include "pc_buffer.h"

#include <stdlib.h>
#include <stdio.h>

#define PC_BUFFER_SIZE 9999
//#define PC_BUFFER_SIZE (1024*1024)-1 / sizeof(struct wrapper)

/* Initialise le buffer ainsi que la place mémoire */
int init_pc_buffer(struct in_packet_buffer ** b) {
	(*b) = (struct in_packet_buffer*) malloc(sizeof (struct in_packet_buffer));
	if(*b == NULL) {
		fprintf(stderr, "Problem to allouate buffer memory\n");
		return 1;
	}
	(*b)->buffer = (struct wrapper* ) malloc(PC_BUFFER_SIZE*sizeof(struct wrapper));
	if((*b)->buffer == NULL) {
		fprintf(stderr, "Problem to allouate buffer memory\n");
		free(*b);
		*b = NULL;
		return 1;
	}

	/* Initialise les sémaphores et le mutex */
	if(pthread_mutex_init( &(*b)->mutex, NULL)) {
		perror("Mutex init");
		free((*b)->buffer);
		(*b)->buffer = NULL;
		free(*b);
		*b = NULL;
		return 1;
	}
	if(sem_init(&(*b)->empty, 0, PC_BUFFER_SIZE-1)) {
		perror("Sem init");
		free((*b)->buffer);
		(*b)->buffer = NULL;
		pthread_mutex_destroy(&(*b)->mutex);
		free(*b);
		*b = NULL;
		return 1;
	}
	if(sem_init(&(*b)->full, 0, 0 )) {
		perror("Sem init");
		free((*b)->buffer);
		(*b)->buffer = NULL;
		pthread_mutex_destroy(&(*b)->mutex);
		sem_destroy(&(*b)->empty);
		free(*b);
		*b = NULL;
		return 1;
	}

	(*b)->readpos = 0;
	(*b)->writepos = 0;
	(*b)->in = 0;

	return 0;
}

int notEmpty(struct in_packet_buffer * b) {
	return b->in != 0;
}

/* Libère le buffer */
void free_pc_buffer(struct in_packet_buffer * b) {
	if(b != NULL) {
		if(b->buffer != NULL) {
			free(b->buffer);
			b->buffer = NULL;
		}

		if( sem_destroy(&b->empty))
			perror("Sem destroy");
		if( sem_destroy(&b->full))
			perror("Sem destroy");
		if(pthread_mutex_destroy(&(b->mutex)))
			perror("Mutex destroy");

		free(b);
		b=NULL;
	}
}

void advance_wr_pos(struct in_packet_buffer *b) {
	b->writepos++;
	if (b->writepos >= PC_BUFFER_SIZE)
		b->writepos = 0;
    b->in++;
}

void advance_rd_pos(struct in_packet_buffer *b) {
	b->readpos++;
	if (b->readpos >= PC_BUFFER_SIZE)
		b->readpos = 0;

	b->in--;
}


void producer_pc_wait(struct in_packet_buffer *b) {
    sem_wait(&(b->empty));
    pthread_mutex_lock(&(b->mutex));
}

void producer_pc_post(struct in_packet_buffer *b) {
    pthread_mutex_unlock(&(b->mutex));
    sem_post(&(b->full));
}

void consumer_pc_wait(struct in_packet_buffer *b) {
    sem_wait(&(b->full));
    pthread_mutex_lock(&(b->mutex));
}

void consumer_pc_post(struct in_packet_buffer *b) {
    pthread_mutex_unlock(&(b->mutex));
    sem_post(&(b->empty));
}


/* Signal que on a finit de put dans le buffer */
void push_ps_finish(struct in_packet_buffer *b) {
	sem_post(&b->full);
}


