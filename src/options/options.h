/**
 * @file options.h
 * @author toriki_l
 * @brief File containing the informations needed for options parsing
 *
 * This file contains the definition of the `options' struct and
 * `parse_options' function.
 *
 */
#ifndef OPTIONS_H
#define OPTIONS_H

/**
 * @brief The structure that stores the options from the command line
 */
struct options
{
    char *file; /**< The name of the file (-f file) */
    char print; /**< Set to one if print option (-p) is enabled */
    char help; /**< Set to one if help option (-h) is enabled */
    unsigned rule_offset; /**< The offset of the first rule in argv */
};

/**
 * @brief Parses the arguments from the command line
 *
 * This function parses the arguments from the command line.
 * The corresponding values are strored in options.
 * Options also store the offset of the first rule in argv.
 *
 * @param argc Argc from the main
 * @param argv[] Argv from the main
 * @param options An allocated options structure where the options are stored
 */
void parse_options(int argc, char *argv[], struct options *options);

#endif /* OPTIONS_H */
