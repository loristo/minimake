#include <parse/parse.h>

struct parsed *g_parsed = NULL;

FILE *get_makefile(const char *file)
{
    (void) file;
    return NULL;
}

void parse(const char *file)
{
    (void) file;
}

void parsed_free(void)
{
    linked_free(&g_parsed->variables);
    linked_free(&g_parsed->rules);
}
