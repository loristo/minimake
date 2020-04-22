/**
 * @file variable.h
 * @author toriki_l
 * @brief File containing definitions needed for variables
 *
 * This file contains the definitions of the `variable' struct and the
 * `variable_assign' and `variable_free' functions.
 *
 */
#ifndef VARIABLE_H
#define VARIABLE_H

/**
 * @brief The struct used to store a variable
 */
struct variable
{
    char *name; /**< The name of the variable */
    char *value; /**< The value of the variable */
};

/**
 * @brief This function assign a value to a variable
 *
 * @param var_name The name of the variable
 * @param var_value The value of the variable
 *
 * @return 1 if the assignment was sucessful, 0 otherwise
 */
int variable_assign(const char *var_name, const char *var_value);

/**
 * @brief This function frees a variable
 *
 * @param variable_ptr The variable to free
 */
void variable_free(void *variable_ptr);

int variable_expand(char **str);

#endif /* VARIABLE_H */
