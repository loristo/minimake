#ifndef OPTIONS_H
#define OPTIONS_H

#define _GNU_SOURCE

struct options
{
    char *file;
    int print;
    int help;
    unsigned rule_offset;
};

void parse_options(int argc, char *argv[], struct options *options);

#endif /* OPTIONS_H */
