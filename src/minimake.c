#include <options/options.h>

#include <stdio.h>

int main(int argc, char *argv[])
{
    struct options opt;
    parse_options(argc, argv, &opt);
    for (unsigned i = 1 + opt.rule_offset; argv[i] != NULL; ++i)
        printf("%s\n", argv[i]);
}
