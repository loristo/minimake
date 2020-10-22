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
    struct linked dependencies; /**< The linked struct containing the list of
                                   dependencies */
    struct linked
        commands; /**< The linked struct containing the list of commands */
    int is_built; /**< 1 if the rule was built, 0 otherwise */
};

/**
 * @brief Ruturn code of the execution
 */
enum exec_return_code
{
    NOTHING = 0,
    BUILT = 1,
    EXEC = 2
};

/**
 * @brief This function assign a value to a variable
 *
 * @param target The name of the target
 * @param dependencies The dependencies
 * @param commands The commands
 *
 * @return 1 if the assignment was successful, 0 otherwise
 */
int rule_assign(char *target, struct linked *dependencies,
                struct linked *commands);

/**
 * @brief This function frees a rule
 *
 * @param rule_ptr The rule to free
 */
void rule_free(void *rule_ptr);

/**
 * @brief This functions executes the target
 *
 * @param targets[] The taget to execute
 */
void exec(char *targets[]);

#endif /* RULE_H */
