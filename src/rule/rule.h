/**
 * @file rule.h
 * @author toriki_l
 * @brief File containing definitions needed for 
 *
 * This file contains the definition of the `rule' struct.
 *
 */
#ifndef RULE_H
#define RULE_H

/**
 * @brief The struct used to store a rule
 */
struct rule
{
    char *target; /**< The name of the target */
    struct linked dependencies; /**< The linked struct containing the list of dependencies */
    struct linked commands; /**< The linked struct containing the list of commands */
};

int rule_assign(const char *target, struct linked *dependencies,
        struct linked *commands);
void rule_free(void *rule_ptr);

#endif /* RULE_H */
