#ifndef PARSE_H
#define PARSE_H

#include <parse/linked.h>

struct variable
{
    char *name;
    char *value;
};

struct rule
{
    char *taget;
    struct linked dependencies;
    struct linked commands;
};

struct parsed
{
    struct linked rules;
    struct linked variables;
};

extern struct parsed *g_parsed;

#endif /* PARSE_H */
