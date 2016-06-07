#include "refrag_buffer.h"

size_t BUFFER_FRAGS_SIZE = 8000;
extern int verbose_flag;

int init_bulk_frag(struct bulk_frag **b, size_t buffer_size, uint32_t frag_id) {

    printf("create a new bulk_frag\n");

    if((*b = (struct bulk_frag *) malloc(sizeof(struct bulk_frag))) == NULL) {
        fprintf(stderr, "bulk_frag cannot be malloc\n");
        return -1;
    }

    if( ((*b)->buffer = (char *) malloc(buffer_size)) == NULL ){
        fprintf(stderr, "Unable to malloc buffer\n");
        return -1;
    }

    if(((*b)->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t))) == NULL) {
        fprintf(stderr, "Unable to allow size for lock\n");
        return -1;
    }

    if(pthread_mutex_init((*b)->lock, NULL) != 0) {
        fprintf(stderr, "Unable to init lock\n");
        return -1;
    }

    (*b)->id = frag_id;
    (*b)->end_pos = BUFFER_FRAGS_SIZE;
    (*b)->wrote_bytes = 0;

    return 0;
}

struct bulk_frag *get_bulk_frag(struct bulk_frag **b_frags, uint32_t frag_id) {
    struct bulk_frag *frag;
    HASH_FIND_INT(*b_frags, &frag_id, frag);
    return frag;
}

struct bulk_frag *add_bulk_frag(struct bulk_frag **b_frags, uint32_t frag_id) {
    struct bulk_frag *frag;
    init_bulk_frag(&frag, BUFFER_FRAGS_SIZE, frag_id);
    HASH_ADD_INT( *b_frags, id, frag);
    return frag;
}

struct bulk_frag *get_bulk_frag_or_create(struct bulk_frag **b_frags, uint32_t frag_id) {
    struct bulk_frag *res;
    if((res = get_bulk_frag(b_frags, frag_id)) == NULL)
        return add_bulk_frag(b_frags, frag_id);

    return res;
}


