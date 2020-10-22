#define _GNU_SOURCE

#include <minimake.h>
#include <options/options.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void parse_options(int argc, char *argv[], struct options *options)
{
    memset(options, 0, sizeof(struct options));
    const char *optstring = "f:ph";
    for (int opt = getopt(argc, argv, optstring); opt != -1;
         opt = getopt(argc, argv, optstring))
    {
        switch (opt)
        {
        case 'f':
            options->file = optarg;
            options->rule_offset += 2;
            break;
        case 'p':
            options->print = 1;
            ++options->rule_offset;
            break;
        case 'h':
            options->help = 1;
            ++options->rule_offset;
            break;
        default:
            exit(ERR_BAD_OPTION);
        }
    }
}
