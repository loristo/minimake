/**
 * @file parse.h
 * @author toriki_l
 * @brief File containing definitions needed for parsing
 *
 * This file contains the definition of the `parsed' struct and the `parse'
 * function.
 *
 */
#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>

#include <parse/linked.h>
#include <variable/variable.h>
#include <rule/rule.h>

#define ERROR_MESSAGE_SIZE 64

/**
 * @brief The struct used to store informations need by the parser
 */
struct parser
{
    char **line; /**< The current line */
    size_t *n; /**< The size of the allocated buffer */
    FILE *file; /**< The file that is parsed */
    int error; /**< The error code in case of erro */
    char error_message[ERROR_MESSAGE_SIZE]; /**< The error message in case of error */
};

/**
 * @brief The struct used to store the rules and variables of the file
 */
struct parsed
{
    struct linked rules; /**< The linked struct containing the list of rules */
    struct linked variables; /**< The linked struct containing the list of variables */
};

/**
 * @brief The function that parses the file
 *
 * The parsed informations are stored in the g_parsed variable.
 * If the file was not found, g_parsed is null.
 *
 * @param filename If null, looks for makefile or Makefile, else the filename
 */
void parse(const char *filename);

#endif /* PARSE_H */
