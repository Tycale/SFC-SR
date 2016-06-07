#ifndef _LIBLFQ_H
#define _LIBLFQ_H

#ifndef __x86_64__
#error "The program is developed for x86-64 architecture only."
#endif
#if !defined(DCACHE1_LINESIZE) || !DCACHE1_LINESIZE
#ifdef DCACHE1_LINESIZE
#undef DCACHE1_LINESIZE
#endif
#define DCACHE1_LINESIZE 64
#endif
#define ____cacheline_aligned   __attribute__((aligned(DCACHE1_LINESIZE)))

#define Q_SIZE  (1024*1024)
#define Q_MASK  (Q_SIZE - 1)

#ifdef MAX
#undef MAX
#endif

#define MAX(p,q) ((p) > (q) ? (p) : (q))


#include "lfq.h"
#include <ctype.h>
#include <stdint.h>
#include <stddef.h>

extern __thread size_t __thr_id;

static inline size_t thr_id(void)
{
    return __thr_id;
}

static inline void set_thr_id(size_t id)
{
    __thr_id = id;
}

struct __thrpos {
    unsigned long head, tail;
};

struct lfqueue {
    size_t n_prod;
    size_t n_cons;
    size_t msize;
    unsigned long head ____cacheline_aligned;
    unsigned long tail ____cacheline_aligned;
    unsigned long last_head ____cacheline_aligned;
    unsigned long last_tail ____cacheline_aligned;

    struct __thrpos *thr_p;
    void *ptr_array;
};

struct lfqueue *lfq_init(int, int, size_t);
void lfq_destroy(struct lfqueue *);
void lfq_push(struct lfqueue *, void *);
void lfq_pop(struct lfqueue *, void *);
struct __thrpos *lfq_thr_pos(struct lfqueue *);

#endif
