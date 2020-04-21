/**
 * @file parse.h
 * @author toriki_l
 * @brief File containing definitions needed for parsing
 *
 * This file contains the definition of the the `rule'
 * struct, the `parsed' struct, the g_parsed global variables and the parse
 * function.
 *
 */
#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>

#include <parse/linked.h>
#include <variable/variable.h>

/**
 * @brief The struct used to store a rule
 */
struct rule
{
    char *taget; /**< The name of the target */
    struct linked dependencies; /**< The linked struct containing the list of dependencies */
    struct linked commands; /**< The linked struct containing the list of commands */
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
