#include <stdio.h>
#include <stdlib.h>

#include <minimake.h>
#include <options/options.h>
#include <parse/parse.h>

struct parsed *g_parsed = NULL;

static void help(void)
{
    printf("Usage: minimake [ -f filename ] [ -h ] [ -p ] [ RULES ... ]\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    struct options opt;
    parse_options(argc, argv, &opt);
    if (opt.help)
        help();
    parse(opt.file);
}
