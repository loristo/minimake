#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <minimake.h>
#include <parse/parse.h>

static void parsed_free(void)
{
    linked_free(&g_parsed->variables, variable_free);
    linked_free(&g_parsed->rules, rule_free);
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

static void parse_rule_var(char **line, size_t *n, FILE *file);

static int generate_dependencies(char *dependencies_str,
        struct linked *dependencies)
{
    if (!dependencies_str)
        return 1;
    const char *whitespaces = " \t\r\n\v\f";
    char *saveptr;
    for (char *token = strtok_r(dependencies_str, whitespaces, &saveptr); token;
        token = strtok_r(NULL, whitespaces, &saveptr))
    {
        if (!linked_strdup(dependencies, token))
            return 0;
    }
    return 1;
}

static int generate_commands(char **line, size_t *n, FILE *file,
        struct linked *commands, ssize_t *i)
{
    const char *whitespaces = " \t\r\n\v\f";
    for (*i = getline(line, n, file); *i != -1 && (*line)[0] == '\t';
            *i = getline(line, n, file))
    {
        if (!linked_strdup(commands, *line + strspn(*line, whitespaces)))
            return 0;
    }
    return 1;
}

static void parse_rule(char **line, size_t *n, FILE *file)
{
    const char *whitespaces = " \t\r\n\v\f";
    char *saveptr;
    char *target = strtok_r(*line, ":", &saveptr);
    char *dependencies_str = strtok_r(NULL, "\n", &saveptr);
    struct linked dependencies = { NULL, NULL };
    struct linked commands = { NULL, NULL };
    target = strtok_r(target, whitespaces, &saveptr);
    if (strtok_r(NULL, whitespaces, &saveptr))
    {
        free(*line);
        fclose(file);
        errx(ERR_NO_RULE_NO_VAR, "*** mutilple rule names.  Stop");
    }
    ssize_t i = 0;
    if (!generate_dependencies(dependencies_str, &dependencies) ||
            !generate_commands(line, n, file, &commands, &i))
    {
        linked_free(&commands, NULL);
        linked_free(&dependencies, NULL);
        free(*line);
        fclose(file);
        errx(ERR_BAD_ALLOC, "*** allocation error.  Stop");
    }
    if (!rule_assign(target, &dependencies, &commands))
    {
        free(*line);
        fclose(file);
        errx(ERR_BAD_ALLOC, "*** allocation error.  Stop");
    }
    if (i != -1)
        parse_rule_var(line, n, file);
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
    const char *whitespaces = " \t\r\n\v\f";
    if (strspn(*line, whitespaces) - strlen(*line) == 0)
        return;
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
    for (ssize_t i = getline(&line, &n, file); i != -1;
            i = getline(&line, &n, file))
    {
        char *comment = strchr(line, '#');
        if (comment)
            *comment = '\n';
        parse_rule_var(&line, &n, file);
    }
    free(line);
    fclose(file);
}
