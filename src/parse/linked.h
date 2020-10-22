/**
 * @file linked.h
 * @author toriki_l
 * @brief File containing linked list definitions
 *
 * This file contains the definition of the `variable' struc, the `rule'
 * struct, the `parsed' struct, the g_parsed global variables and the parse
 * functions.
 *
 */
#ifndef LINKED_H
#define LINKED_H

#include <stddef.h>

/**
 * @brief A simple linked list
 */
struct _linked
{
    struct _linked *next; /**< The next element of the linked list */
    void *data; /**< The contained data */
};

/**
 * @brief A header for the linked list containing the head and the tail of the
 * linked list
 */
struct linked
{
    struct _linked *head; /**< The head of the linked list */
    struct _linked *tail; /**< The tail of the linked list */
};

/**
 * @brief This function allocates a new entry with a data of size n at the
 * end of the a linked list
 *
 * @param l The link list header in wich the entry is added
 * @param n The size of the data to allocate
 *
 * @return The pointer to the allocated data
 */
void *linked_allocate(struct linked *l, size_t n);

/**
 * @brief This function allocates a new entry and copies the content of src
 * to the data
 *
 * @param l The link list header in wich the entry is added
 * @param src The string that will be copied
 *
 * @return The pointer to the allocated string
 */
char *linked_strdup(struct linked *l, char *src);

/**
 * @brief This functions copies a link linst comataining strings
 *
 * @param dst The destination
 * @param src The source
 *
 * @return Returns 1 if successfull, 0 otherwise
 */
int linked_str_copy(struct linked *dst, const struct linked *src);

/**
 * @brief This functions frees the linked list l
 *
 * @param l The linked list that will be freed
 * @param free_func The function used to free the data
 */
void linked_free(struct linked *l, void (*free_func)(void *));

#endif /* LINKED_H */
