#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <minimake.h>
#include <variable/variable.h>

static struct variable *variable_search(const char *var_name)
{
    struct variable *variable;
    for (struct _linked *l = g_parsed->variables.head; l; l = l->next)
    {
        variable = l->data;
        if (!strcmp(var_name, variable->name))
            return variable;
    }
    return NULL;
}

static char **variable_get_value(const char *var_name, size_t n,
        enum variable_status **status)
{
    char *s = malloc(n + 1);
    if (!s)
        return NULL;
    strncpy(s, var_name, n);
    s[n] = 0;
    struct variable *variable = variable_search(s);
    if (!variable || variable->is_env)
    {
        if (!variable_assign(s, getenv(s)))
        {
            free(s);
            return NULL;
        }
        variable = variable_search(s);
        variable->is_env = 1;
    }
    if (status)
        *status = &variable->status;
    free(s);
    return &variable->value;
}

int variable_assign(const char *var_name, const char *var_value)
{
    struct variable *variable = variable_search(var_name);
    if (!variable)
    {
        variable = linked_allocate(&g_parsed->variables,
                sizeof(struct variable));
        if (!variable)
            return 0;
        variable->name = strdup(var_name);
        if (!variable->name)
            return 0;
    }
    else
        free(variable->value);
    variable->value = strdup(var_value ? var_value : "");
    if (!variable->value)
        return 0;
    variable->status = NOT_PROCESSED;
    variable->is_env = 0;
    return 1;
}

int variable_replace(char **str, char *start, size_t size,
        const char *token)
{
    if (!strncmp(start, "$$", 2))
        token = "$";
    const size_t len = strlen(token);
    const int diff = len - size;
    const size_t move = strlen(start + size) + 1;
    const size_t offset = start - *str;
    const size_t next_size = strlen(*str) + diff + 1;
    if (!*str)
        return 0;
    if (diff > 0)
    {
        *str = realloc(*str, next_size);
        start = *str + offset;
        memmove(start + len, start + size, move);
    }
    else
    {
        memmove(start + len, start + size, move);
        *str = realloc(*str, next_size);
        start = *str + offset;
    }
    memcpy(start, token, len);
    return 1;
}

int variable_expand(char **str, int persistent)
{
    size_t size;
    int res;
    char **var;
    char *s = NULL;
    size_t offset = 0;
    enum variable_status *status;
    // for each '$' in line
    for (char *variable_ptr = strchr(*str, '$'); variable_ptr;
            variable_ptr = strchr(*str + offset + 1, '$'))
    {
        // gets variable
        size = 2;
        offset = variable_ptr - *str;
        if (variable_ptr[1] == '\n' || variable_ptr[1] == '\0')
            return 0;
        else if (variable_ptr[1] == '{' || variable_ptr[1] == '(')
        {
            if (variable_ptr[1] == '{')
            {
                size = strcspn(variable_ptr, "}") + 1;
                if (*(variable_ptr + size - 1) != '}')
                    return 2;
            }
            else
            {
                size = strcspn(variable_ptr, ")") + 1;
                if (*(variable_ptr + size - 1) != ')')
                    return 2;
            }
            var = variable_get_value(variable_ptr + 2, size - 3, &status);
        }
        else
            var = variable_get_value(variable_ptr + 1, 1, &status);

        // expands its content
        if (!var)
            return 1;
        if (!persistent)
        {
            s = strdup(*var);
            if (!s)
                return 1;
            var = &s;
        }
        switch (*status)
        {
            case PROCESSING:
                if (!persistent)
                    free(s);
                return 3;
            case NOT_PROCESSED:
                *status = PROCESSING;
                res = variable_expand(var, persistent);
                *status = persistent ? PROCESSED : NOT_PROCESSED;
                break;
            default:
                res = 0;
                break;
        }

        // expand in str
        if (!variable_replace(str, variable_ptr, size, *var))
            res = 1;
        free(s);
        if (res || (*str)[offset] == '\0')
            return res;
    }
    return 0;
}

void variable_free(void *variable_ptr)
{
    struct variable *variable = variable_ptr;
    free(variable->value);
    free(variable->name);
    free(variable);
}
