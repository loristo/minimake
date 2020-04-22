/**
 * @file minimake.h
 * @author toriki_l
 * @brief File containing general macros for minimake
 *
 * This file contains the definition of usefull defines and the global variable
 *
 */
#ifndef MINIMAKE_H
#define MINIMAKE_H

#include <parse/parse.h>

#define ERR_BAD_OPTION 2
#define ERR_NO_FILE 2
#define ERR_BAD_ALLOC 2
#define ERR_NO_RULE_NO_VAR 2
#define ERR_MULTIPLE_VAR_NAME 2
#define ERR_BAD_VAR 2
#define ERR_RECURSIVE_VAR 2

/**
 * @brief The global variable containing the parsed informations
 */
extern struct parsed *g_parsed;

#endif /* MINIMAKE_H */
