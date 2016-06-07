#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <assert.h>
#include <xmmintrin.h>
#include "lfq.h"

//__thread size_t __thr_id;

struct __thrpos *lfq_thr_pos(struct lfqueue *q)
{
    assert(thr_id() < MAX(q->n_prod, q->n_cons));
    return &q->thr_p[thr_id()];
}

struct lfqueue *lfq_init(int prod, int cons, size_t msize)
{
    struct lfqueue *q;
    int n;

    q = malloc(sizeof(*q));
    memset(q, 0, sizeof(*q));

    q->n_prod = prod;
    q->n_cons = cons;
    q->msize = msize;

    n = MAX(prod, cons);

    if ((posix_memalign((void **)&q->thr_p, getpagesize(), n*sizeof(struct __thrpos))) < 0) {
        perror("posix_memalign");
        free(q);
        return NULL;
    }
    memset(q->thr_p, 0xff, n*sizeof(struct __thrpos));

    if ((posix_memalign((void **)&q->ptr_array, getpagesize(), Q_SIZE*msize)) < 0) {
        perror("posix_memalign");
        free(q->thr_p);
        free(q);
        return NULL;
    }

    return q;
}

void lfq_destroy(struct lfqueue *q)
{
    free(q->ptr_array);
    free(q->thr_p);
    free(q);
}

void lfq_push(struct lfqueue *q, void *item)
{
    lfq_thr_pos(q)->head = q->head;
    lfq_thr_pos(q)->head = __sync_fetch_and_add(&q->head, 1);

    while (__builtin_expect(lfq_thr_pos(q)->head >= q->last_tail + Q_SIZE, 0)) {
        size_t i;
        unsigned long min = q->tail;

        for (i = 0; i < q->n_cons; ++i) {
            unsigned long tmp_t = q->thr_p[i].tail;

            asm volatile("" ::: "memory");

            if (tmp_t < min)
                min = tmp_t;
        }
        q->last_tail = min;

        if (lfq_thr_pos(q)->head < q->last_tail + Q_SIZE)
            break;

        _mm_pause();
    }

    memcpy(q->ptr_array + (lfq_thr_pos(q)->head & Q_MASK)*q->msize, item, q->msize);

    lfq_thr_pos(q)->head = ULONG_MAX;
}

void lfq_pop(struct lfqueue *q, void *item)
{
    lfq_thr_pos(q)->tail = q->tail;
    lfq_thr_pos(q)->tail = __sync_fetch_and_add(&q->tail, 1);

    while (__builtin_expect(lfq_thr_pos(q)->tail >= q->last_head, 0)) {
        size_t i;
        unsigned long min = q->head;

        for (i = 0; i < q->n_prod; ++i) {
            unsigned long tmp_h = q->thr_p[i].head;

            asm volatile("" ::: "memory");

            if (tmp_h < min)
                min = tmp_h;
        }
        q->last_head = min;

        if (lfq_thr_pos(q)->tail < q->last_head)
            break;

        _mm_pause();
    }

    memcpy(item, q->ptr_array + (lfq_thr_pos(q)->tail & Q_MASK)*q->msize, q->msize);

    lfq_thr_pos(q)->tail = ULONG_MAX;
}
