#include <semaphore.h>
#include <pthread.h>


#ifndef PC_BUFFER_H
#define PC_BUFFER_H

struct wrapper {
    struct nlattr **attrs;
    struct nlmsghdr *nlh;
} wrapper;

struct in_packet_buffer {
	struct wrapper* buffer;
	sem_t full;
	sem_t empty;
	pthread_mutex_t mutex;
	int readpos;
	int writepos;
	int in;
}in_packet_buffer;

/* Initialise le buffer ainsi que la place mémoire */
int init_pc_buffer(struct in_packet_buffer ** b);

/* Libère le buffer */
void free_pc_buffer(struct in_packet_buffer * b);


void consumer_pc_post(struct in_packet_buffer *b);
void consumer_pc_wait(struct in_packet_buffer *b);
void producer_pc_post(struct in_packet_buffer *b);
void producer_pc_wait(struct in_packet_buffer *b);



/* Signal que on a finit de put dans le buffer */
void push_pc_finish(struct in_packet_buffer *b);

int notEmpty(struct in_packet_buffer * b);


#endif
