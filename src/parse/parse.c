#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <minimake.h>
#include <parse/parse.h>

struct parsed *g_parsed = NULL;

static void parsed_free(void)
{
    linked_free(&g_parsed->variables);
    linked_free(&g_parsed->rules);
    free(g_parsed);
}

static FILE *get_makefile(const char *filename)
{
    FILE *res;
    if (filename)
        res = fopen(filename, "r");
    else
    {
        res = fopen("makefile", "r");
        if (!res)
            res = fopen("Makefile", "r");
    }
    return res;
}

static void parse_rule(char **line, size_t *n, FILE *file)
{
    (void) line;
    (void) n;
    (void) file;
}

static void parse_var(char **line, size_t *n, FILE *file)
{
    (void) line;
    (void) n;
    (void) file;
}

static void parse_rule_var(char **line, size_t *n, FILE *file)
{
    for (size_t i = 0; i < *n && (*line)[i]; ++i)
    {
        if ((*line)[i] == ':')
        {
            parse_rule(line, n, file);
            return;
        }
        else if ((*line)[i] == '=')
        {
            parse_var(line, n, file);
            return;
        }
        else
            continue;
    }
    free(*line);
    fclose(file);
    errx(ERR_NO_RULE_NO_VAR, "*** missing separator.  Stop");
}

void parse(const char *filename)
{
    FILE *file = get_makefile(filename);
    if (!file)
    {
        filename
            ? err(ERR_NO_FILE, "%s", filename)
            : errx(ERR_NO_FILE, "*** No makefile found");
    }
    g_parsed = calloc(1, sizeof(struct parsed));
    if (!g_parsed)
        exit(ERR_BAD_ALLOC);
    atexit(parsed_free);
    char *line = NULL;
    size_t n = 0;
    for (ssize_t i = getline(&line, &n, file); i != -1; i = getline(&line, &n, file))
        parse_rule_var(&line, &n, file);
    free(line);
    fclose(file);
}
