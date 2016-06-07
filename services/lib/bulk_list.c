
#include "bulk_list.h"


struct bulk_buffer *glob_buffer;


extern size_t BUFFER_SIZE;
int MIN_CONS_DATA=7000;

void show_list(struct el_bulk_list *list) {
    struct el_bulk_list *el;
    //printf("List: [");
    LL_FOREACH(list,el) printf("%u (%d), ", el->seq, el->tcp->pkt_len);
    //printf("]\n");
}

struct el_bulk_list *insert_tcp_to_list(struct seg6_sock *sk, struct bulk_list *list,
        struct el_bulk_list *el) {
    struct el_bulk_list *tmp, *pivot=NULL;
    int cons_data;

    el->next = NULL;
    gettimeofday(&(el->last_send), NULL);

    tmp = list->list;

    if(tmp != NULL)
        cons_data = tmp->len;
    else
        cons_data = 0;

    while(tmp != NULL && tmp->next != NULL && tmp->next->seq < el->seq) {

        if(tmp->seq + tmp-> len == tmp->next->seq)
            cons_data = tmp->next->len;
        else
            cons_data += tmp->next->len;

        tmp = tmp->next;

        if(cons_data >= MIN_CONS_DATA)
            pivot = tmp;
    }

    if(tmp != NULL && tmp->seq > el->seq)
        LL_PREPEND_ELEM(list->list, tmp, el);
    else
        LL_APPEND_ELEM(list->list, tmp, el);

    return pivot;
}


int init_bulk_list(struct bulk_list **b) {
    if((*b = (struct bulk_list *) malloc(sizeof(struct bulk_list))) == NULL) {
        fprintf(stderr, "bulk_list cannot be malloc\n");
        return -1;
    }

    (*b)->list = NULL;
    (*b)->lock = NULL;

    if(((*b)->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t))) == NULL) {
        fprintf(stderr, "Unable to allow size for lock\n");
        free_bulk_list(*b);
        return -1;
    }

    if(pthread_mutex_init((*b)->lock, NULL) != 0) {
        fprintf(stderr, "Unable to init lock\n");
        free_bulk_list(*b);
        return -1;
    }

    return 0;
}

/*
 * Free bulk_list
 */
void free_bulk_list(struct bulk_list *b) {
    if(b != NULL){
        if(b->lock != NULL) {
            pthread_mutex_destroy(b->lock);
            free(b->lock);
        }
        free_list(&(b->list));
        free(b);
    }
}

void free_list(struct el_bulk_list **list) {
    struct el_bulk_list *el, *tmp;

    LL_FOREACH_SAFE(*list, el, tmp) {
      LL_DELETE(*list, el);
      free(el->tcp);
      free(el);
    }
}

struct el_bulk_list *find_pivot(struct bulk_list *list, int ms_delta) {

    struct el_bulk_list *res=NULL, *tmp = list->list;
    struct timeval now;

    gettimeofday(&now, NULL);

    while(tmp != NULL ) {
        if(timevaldiff(&(tmp->last_send), &now) >= ms_delta)
            res = tmp;
        tmp = tmp->next;
    }

    return res;
}

void send_list_until(struct seg6_sock *sk, struct bulk_list *bulk_list,
        struct el_bulk_list *pivot, int flags) {
    struct el_bulk_list *el, *added;
    uint32_t seq;
    uint32_t opt;

    el = bulk_list->list;
    bulk_list->list = pivot;

    while(el != NULL && el != pivot ) {
        insert_tcp_to_buffer(sk, glob_buffer, el->tcp, &seq, &opt);
        added = el;
        el = el->next;

        free(added->tcp);
        free(added);
    }

    send_buffer(sk, glob_buffer, flags);
}

void send_old_list(struct seg6_sock *sk, struct bulk_list *list, int ms_delta, int flags) {
    struct el_bulk_list *pivot= NULL;

    //FIXME
    pthread_mutex_lock(list->lock);
    //if(pthread_mutex_trylock(list->lock) == 0) {
        pivot = find_pivot(list, ms_delta);

        if(pivot != NULL)
            send_list_until(sk, list, pivot, flags);

        pthread_mutex_unlock(list->lock);
    //}
}



void send_list(struct seg6_sock *sk, struct bulk_list *list, int flags) {
    send_list_until(sk, list, NULL, flags);
}
