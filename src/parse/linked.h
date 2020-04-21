#ifndef LINKED_H
#define LINKED_H

#include <stddef.h>

struct _linked
{
    struct _linked *next;
    void *data;
};

struct linked
{
    struct _linked *head;
    struct _linked *tail;
};

void *linked_allocate(struct linked *l, size_t n);
void linked_free(struct linked *l);

#endif /* LINKED_H */
