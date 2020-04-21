/**
 * @file variable.h
 * @author toriki_l
 * @brief File containing definitions needed for variables
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

int variable_assign(const char *var_name, const char *var_value);
void variable_free(void *variable_ptr);

#endif /* VARIABLE_H */
