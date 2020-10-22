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
 * @brief The processing status of a variable
 */
enum variable_status
{
    PROCESSED,
    PROCESSING,
    NOT_PROCESSED
};

/**
 * @brief The struct used to store a variable
 */
struct variable
{
    char *name; /**< The name of the variable */
    char *value; /**< The value of the variable */
    enum variable_status status; /**< The processing status of the variable */
    int is_env; /**< If the variable is from ENV */
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

/**
 * @brief This function expands the variables in the string passed
 *
 * The string might be realloced during the process. This function handles
 * the possible error and returns the code.
 *
 * @param str The string in which the variables are expanded
 * @param persistent If 0, the variables changes are not persistent
 *
 * @return The error code or 0 if no error occured
 */
int variable_expand(char **str, int persistent);

/**
 * @brief Replaces the size bytes from start with token in the given string
 *
 * @param str A pointer to the string to modify
 * @param start The start of the variable in the string
 * @param size The number of bytes to replace
 * @param token The token that will added in the string
 *
 * @return 1 if the expand was sucessful, 0 otherwise
 */
int variable_replace(char **str, char *start, size_t size, const char *token);

#endif /* VARIABLE_H */
