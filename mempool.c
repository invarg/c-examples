/*
 * mempool.c  -  fixed-block memory pool (no malloc, no fragmentation)
 *
 * Hands out equal-sized blocks from a static buffer. A free block stores
 * the "next free" pointer inside itself, so alloc/free are O(1) and use
 * no extra memory.
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic mempool.c
 */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct block { struct block *next; } block_t;

typedef struct {
    block_t *free_list;
    size_t   free_count;
} pool_t;

/* Carve `buf` into `count` blocks of `bsize` bytes and link them.
 * bsize must be >= sizeof(void*); buf must outlive the pool. */
void pool_init(pool_t *p, void *buf, size_t bsize, size_t count)
{
    uint8_t *mem = (uint8_t *)buf;
    p->free_list = NULL;
    p->free_count = count;
    for (size_t i = count; i-- > 0u; ) {           /* link block 0 last -> head */
        block_t *b = (block_t *)(void *)(mem + i * bsize);
        b->next = p->free_list;
        p->free_list = b;
    }
}

void *pool_alloc(pool_t *p)
{
    block_t *b = p->free_list;
    if (b == NULL) return NULL;                    /* pool exhausted */
    p->free_list = b->next;
    p->free_count--;
    return b;
}

void pool_free(pool_t *p, void *ptr)
{
    if (ptr == NULL) return;
    block_t *b = (block_t *)ptr;
    b->next = p->free_list;
    p->free_list = b;
    p->free_count++;
}

typedef struct { uint8_t count, command, len, data[16]; } msg_t;
#define N 4u

int main(void)
{
    static msg_t storage[N];                       /* static -> no heap */
    pool_t pool;
    pool_init(&pool, storage, sizeof(msg_t), N);
    printf("free: %zu\n", pool.free_count);

    msg_t *m[N];
    for (unsigned i = 0u; i < N; ++i) m[i] = pool_alloc(&pool);
    printf("after %u allocs, free: %zu\n", N, pool.free_count);
    printf("5th alloc: %s\n", pool_alloc(&pool) ? "ptr" : "NULL (exhausted)");

    pool_free(&pool, m[1]);
    printf("after 1 free, free: %zu\n", pool.free_count);
    (void)m;
    return 0;
}
