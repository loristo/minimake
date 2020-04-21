#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <minimake.h>
#include <parse/parse.h>

static void parsed_free(void)
{
    linked_free(&g_parsed->variables, variable_free);
    linked_free(&g_parsed->rules, NULL);
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
    (void) n;
    const char *whitespaces = " \t\r\n\v\f";
    char *saveptr;
    char *var_name = strtok_r(*line, "=", &saveptr);
    char *var_value = strtok_r(NULL, "\n", &saveptr);
    var_name = strtok_r(var_name, whitespaces, &saveptr);
    if (strtok_r(NULL, whitespaces, &saveptr))
    {
        free(*line);
        fclose(file);
        errx(ERR_NO_RULE_NO_VAR, "*** mutilple variable names.  Stop");
    }
    var_value = var_value + strspn(var_value, whitespaces);
    if (!variable_assign(var_name, var_value))
    {
        free(*line);
        fclose(file);
        errx(ERR_BAD_ALLOC, "*** allocation error.  Stop");
    }

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
    {
        char *comment = strchr(line, '#');
        if (comment)
            *comment = '\n';
        parse_rule_var(&line, &n, file);
    }
    free(line);
    fclose(file);
}
