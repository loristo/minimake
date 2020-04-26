/**
 * @file error.h
 * @author toriki_l
 * @brief File containing the informations needed for error handling
 *
 * This file contains the definition of the `error' struct and
 * `exit_on_error' and `error_init' functions.
 *
 */
#ifndef ERROR_H
#define ERROR_H

#define ERROR_MESSAGE_SIZE 64

/**
 * @brief The structures that contains error informations
 */
struct error
{
    int code; /**< The error code */
    char msg[ERROR_MESSAGE_SIZE]; /**< The error message buffer */
};

/**
 * @brief Sets the error
 *
 * @param error The error struct
 * @param status The error return code
 * @param str The error message
 *
 * @return Always returns 0
 */
int exit_on_error(struct error *error, int status, const char *str);

/**
 * @brief Inits the error structure
 *
 * @param error The error structure that will be initialized
 */
void error_init(struct error *error);

#endif /* ERROR_H */
