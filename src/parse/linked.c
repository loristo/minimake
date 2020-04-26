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
    char *dest = linked_allocate(l, strlen(src) + 1);
    return dest ? strcpy(dest, src) : NULL;
}

void linked_free(struct linked *l, void (*free_func)(void *))
{
    struct _linked *current = l->head;
    struct _linked *next;
    while (current)
    {
        next = current->next;
        free_func ? free_func(current->data) : free(current->data);
        free(current);
        current = next;
    }
}

int linked_str_copy(struct linked *dst, const struct linked *src)
{
    memset(dst, 0, sizeof(struct linked));
    for (struct _linked *l = src->head; l; l = l->next)
        if (!linked_strdup(dst, l->data))
            return 0;
    return 1;
}
