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

static void print(void)
{
    printf("# variables\n");
    struct variable *variable;
    for (struct _linked *variables = g_parsed->variables.head; variables;
            variables = variables->next)
    {
        variable = variables->data;
        printf("'%s' = '%s'\n", variable->name, variable->value);
    }

    printf("# rules\n");
    struct rule *rule;
    for (struct _linked *rules = g_parsed->rules.head; rules;
            rules = rules->next)
    {
        rule = rules->data;
        printf("(%s):", rule->target);
        char *str;
        for (struct _linked *dependencies = rule->dependencies.head;
                dependencies; dependencies = dependencies->next)
        {
            str = dependencies->data;
            printf(" [%s]", str);
        }
        printf("\n");
        for (struct _linked *commands = rule->commands.head;
                commands; commands = commands->next)
        {
            str = commands->data;
            printf("\t'%s'\n", str);
        }
    }
}

int main(int argc, char *argv[])
{
    struct options opt;
    parse_options(argc, argv, &opt);
    if (opt.help)
        help();
    parse(opt.file);
    if (opt.print)
        print();
}
