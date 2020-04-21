#include <stdlib.h>
#include <string.h>

#include <parse/linked.h>

void *linked_allocate(struct linked *l, size_t n)
{
    struct _linked *new = malloc(sizeof(struct _linked));
    if (!new)
        return NULL;
    if (!l->head)
    {
        l->head = new;
        l->tail = new;
    }
    else
    {
        l->tail->next = new;
        l->tail = new;
    }
    new->next = NULL;
    new->data = malloc(n);
    return new->data;
}

char *linked_strdup(struct linked *l, char *src)
{
    char *dest = linked_allocate(l, strlen(src));
    return dest ? strcpy(dest, src) : NULL;
}

void linked_free(struct linked *l)
{
    struct _linked *current = l->head;
    struct _linked *next;
    while (current)
    {
        next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
}
