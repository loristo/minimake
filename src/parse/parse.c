#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <minimake.h>
#include <parse/parse.h>

void parsed_free(void)
{
    if (!g_parsed)
        return;
    linked_free(&g_parsed->variables, variable_free);
    linked_free(&g_parsed->rules, rule_free);
    free(g_parsed);
    g_parsed = NULL;
}

static void parser_fill(struct parser *parser, char **line, size_t *n,
        FILE *file)
{
    parser->n = n;
    parser->line = line;
    parser->file = file;
    parser->error = 0;
    parser->error_message[0] = '\0';
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

static int exit_on_error(struct parser *parser, int status, const char *str)
{
    free(*parser->line);
    fclose(parser->file);
    parser->error = status;
    strncpy(parser->error_message, str, ERROR_MESSAGE_SIZE);
    return 0;
}

static int parse_rule_var(struct parser *parser);

static int parser_expand(char **str, struct parser *parser)
{
    int res = variable_expand(str, 0);
    switch (res)
    {
        case 0:
            break;
        case 1:
            exit_on_error(parser, ERR_BAD_ALLOC,
                "*** allocation error.  Stop");
            __attribute__((fallthrough));
        case 2:
            exit_on_error(parser, ERR_BAD_VAR,
                "*** unterminated variable reference.  Stop");
            __attribute__((fallthrough));
        case 3:
            exit_on_error(parser, ERR_RECURSIVE_VAR,
                "*** Recursive variable references itself (eventually)."
                "  Stop.");
            __attribute__((fallthrough));
        default:
            free(*str);
            return 0;
    }
    return 1;
}

static int parse_dependencies(struct parser *parser ,char *dependencies_str,
        struct linked *dependencies)
{
    const char *whitespaces = " \t\r\n\v\f";
    char *saveptr;
    if (!parser_expand(&dependencies_str, parser))
        return 0;
    for (char *token = strtok_r(dependencies_str, whitespaces, &saveptr); token;
        token = strtok_r(NULL, whitespaces, &saveptr))
    {
        if (!linked_strdup(dependencies, token))
        {
            free(dependencies_str);
            return exit_on_error(parser, ERR_NO_RULE_NO_VAR,
                    "*** allocation error.  Stop");
        }
    }
    free(dependencies_str);
    return 1;
}

static int parse_commands(struct parser *parser, struct linked *commands,
        ssize_t *i)
{
    const char *whitespaces = " \t\r\n\v\f";
    for (*i = getline(parser->line, parser->n, parser->file);
            *i != -1 && ((*parser->line)[0] == '\t' ||
                (*parser->line)[0] == '#' || (*parser->line)[0] == '\n');
            *i = getline(parser->line, parser->n, parser->file))
    {
        if ((*parser->line)[0] == '#' || (*parser->line)[0] == '\n')
            continue;
        (*parser->line)[*i - 1] = '\0';
        if (!linked_strdup(commands, *parser->line + strspn(*parser->line,
                        whitespaces)))
        {
            return exit_on_error(parser, ERR_BAD_ALLOC,
                    "*** allocation error.  Stop");
        }
    }
    if (*i != -1)
    {
        char *comment = strchr(*parser->line, '#');
        if (comment)
            *comment = '\0';
    }
    return 1;
}

static char *remove_spaces(char *str)
{
    const char *whitespaces = " \t\r\n\v\f";
    size_t nb_spaces = strspn(str, whitespaces);
    return memmove(str, str + nb_spaces, strlen(str + nb_spaces) + 1);
}

static int parse_rule(struct parser *parser)
{
    const char *whitespaces = " \t\r\n\v\f";
    char *saveptr;
    char *target;
    char *dependencies_str;
    if (**parser->line == ':')
    {
        target = strdup("");
        dependencies_str = strtok_r(NULL, ":\n", &saveptr);
    }
    else
    {
        target = strdup(strtok_r(*parser->line, ":", &saveptr));
        dependencies_str = strtok_r(NULL, "\n", &saveptr);
    }
    dependencies_str = strdup(dependencies_str ? dependencies_str : "");
    if (!target || !dependencies_str)
    {
        free(target);
        free(dependencies_str);
        return exit_on_error(parser, ERR_BAD_ALLOC,
                "*** allocation error.  Stop");
    }
    if (!parser_expand(&target, parser))
    {
        free(dependencies_str);
        return 0;
    }
    target = remove_spaces(target);
    if (!target || !dependencies_str)
    {
        free(target);
        free(dependencies_str);
        return exit_on_error(parser, ERR_BAD_ALLOC,
                "*** allocation error.  Stop");
    }
    if (strtok_r(target, whitespaces, &saveptr) &&
            strtok_r(NULL, whitespaces, &saveptr))
    {
        free(target);
        free(dependencies_str);
        return exit_on_error(parser, ERR_NO_RULE_NO_VAR,
                "*** multiple rule names.  Stop");
    }
    struct linked dependencies = { NULL, NULL };
    struct linked commands = { NULL, NULL };
    ssize_t i = 0;
    if (!parse_dependencies(parser, dependencies_str, &dependencies) ||
            !parse_commands(parser,  &commands, &i))
    {
        free(target);
        linked_free(&commands, NULL);
        linked_free(&dependencies, NULL);
        return 0;
    }
    if (!rule_assign(target, &dependencies, &commands))
    {
        return exit_on_error(parser, ERR_BAD_ALLOC,
                "*** allocation error.  Stop");
    }
    if (i != -1)
        return parse_rule_var(parser);
    return 1;
}

static int parse_var(struct parser *parser)
{
    const char *whitespaces = " \t\r\n\v\f";
    char *saveptr;
    char *var_name = strtok_r(*parser->line, "=", &saveptr);
    char *var_value = strtok_r(NULL, "\n", &saveptr);
    var_name = strtok_r(var_name, whitespaces, &saveptr);
    if (strtok_r(NULL, whitespaces, &saveptr))
    {
        return exit_on_error(parser, ERR_NO_RULE_NO_VAR,
                "*** multiple variable names.  Stop");
    }
    var_value = var_value + strspn(var_value, whitespaces);
    if (!variable_assign(var_name, var_value))
    {
        return exit_on_error(parser, ERR_NO_RULE_NO_VAR,
                "*** allocation error.  Stop");
    }
    return 1;
}

static int parse_rule_var(struct parser *parser)
{
    const char *whitespaces = " \t\r\n\v\f";
    if (strspn(*parser->line, whitespaces) == strlen(*parser->line))
        return 1;
    size_t sep = strcspn(*parser->line, ":=\n");
    if ((*parser->line)[sep] == ':')
        return parse_rule(parser);
    if ((*parser->line)[sep] == '=')
        return parse_var(parser);
    return exit_on_error(parser, ERR_NO_RULE_NO_VAR,
        "*** missing separator.  Stop");
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
    struct parser parser;
    parser_fill(&parser, &line, &n, file);
    for (ssize_t i = getline(&line, &n, file); i != -1;
            i = getline(&line, &n, file))
    {
        char *comment = strchr(line, '#');
        if (comment)
            *comment = '\0';
        if (!parse_rule_var(&parser))
            errx(parser.error, parser.error_message);
    }
    free(line);
    fclose(file);
}
