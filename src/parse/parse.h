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

#include <error/error.h>
#include <parse/linked.h>
#include <rule/rule.h>
#include <stdio.h>
#include <variable/variable.h>

/**
 * @brief The struct used to store informations need by the parser
 */
struct parser
{
    char **line; /**< The current line */
    size_t *n; /**< The size of the allocated buffer */
    FILE *file; /**< The file that is parsed */
    struct error err; /**< The error reported during parsing */
};

/**
 * @brief The struct used to store the rules and variables of the file
 */
struct parsed
{
    struct linked rules; /**< The linked struct containing the list of rules */
    struct linked pattern_rules; /**< The linked struct containing the list of
                                    pattern rules */
    struct linked
        variables; /**< The linked struct containing the list of variables */
    struct linked *phony; /**< The linked struct containing the phony rules  */
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

/**
 * @brief This functions frees the global variable
 */
void parsed_free(void);

#endif /* PARSE_H */
