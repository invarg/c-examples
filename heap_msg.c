/*
 * heap_msg.c  -  dynamic memory with malloc / realloc / free
 *
 * A growable list of variable-length messages. Each message owns a
 * heap copy of its data; the list owns a heap array of messages.
 * Every allocation is checked, and everything is freed (no leaks).
 *
 * Build: gcc -std=c11 -Wall -Wextra -Wpedantic heap_msg.c
 */
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t  count, command, len;
    uint8_t *data;          /* heap-owned, `len` bytes */
} msg_t;

typedef struct {
    msg_t  *items;          /* heap array, `cap` slots */
    size_t  len, cap;
} msg_list_t;

/* Append a message, deep-copying its data. Returns 0 on success, -1 on
 * out-of-memory (the list is left unchanged so the caller can recover). */
int list_push(msg_list_t *L, uint8_t count, uint8_t cmd,
              const uint8_t *data, uint8_t len)
{
    /* Grow the array geometrically when full. */
    if (L->len == L->cap) {
        size_t new_cap = (L->cap == 0u) ? 4u : L->cap * 2u;
        /* realloc into a temp: if it fails, the original block survives. */
        msg_t *tmp = realloc(L->items, new_cap * sizeof *tmp);
        if (tmp == NULL) return -1;
        L->items = tmp;
        L->cap   = new_cap;
    }

    /* Deep-copy the payload. */
    uint8_t *copy = NULL;
    if (len > 0u) {
        copy = malloc(len);
        if (copy == NULL) return -1;   /* array already grown, still valid */
        memcpy(copy, data, len);
    }

    L->items[L->len].count   = count;
    L->items[L->len].command = cmd;
    L->items[L->len].len     = len;
    L->items[L->len].data    = copy;
    L->len++;
    return 0;
}

/* Free every payload, then the array itself, then reset the struct. */
void list_free(msg_list_t *L)
{
    for (size_t i = 0u; i < L->len; ++i) {
        free(L->items[i].data);
    }
    free(L->items);
    L->items = NULL;
    L->len = L->cap = 0u;
}

int main(void)
{
    msg_list_t list = {0};         /* items=NULL, len=cap=0 */

    const uint8_t d1[] = {0xDE, 0xAD, 0xBE, 0xEF};
    const uint8_t d2[] = {0x7F};
    if (list_push(&list, 0x21, 0x30, d1, sizeof d1) != 0 ||
        list_push(&list, 0x22, 0x31, d2, sizeof d2) != 0 ||
        list_push(&list, 0x23, 0x32, NULL, 0)       != 0) {
        fprintf(stderr, "out of memory\n");
        list_free(&list);
        return 1;
    }

    printf("%zu messages (cap %zu)\n", list.len, list.cap);
    for (size_t i = 0u; i < list.len; ++i) {
        msg_t *m = &list.items[i];
        printf("  count=0x%02X cmd=0x%02X data=[", m->count, m->command);
        for (uint8_t j = 0u; j < m->len; ++j)
            printf("%02X%s", m->data[j], j + 1u < m->len ? " " : "");
        printf("]\n");
    }

    list_free(&list);              /* releases everything */
    printf("freed, no leaks\n");
    return 0;
}
