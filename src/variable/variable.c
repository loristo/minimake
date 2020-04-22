#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <err.h>

#include <minimake.h>
#include <variable/variable.h>

static struct variable *variable_searchn(const char *var_name, size_t n)
{
    struct variable *variable;
    for (struct _linked *l = g_parsed->variables.head; l; l = l->next)
    {
        variable = l->data;
        if (!strncmp(var_name, variable->name, n))
            return variable;
    }
    return NULL;
}

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

static const char *variable_get_value(const char *var_name, size_t n)
{
    struct variable *variable = variable_searchn(var_name, n);
    return variable ? variable->value : "";
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
    variable->value = strdup(var_value);
    if (!variable->value)
        return 0;
    return 1;
}

static int variable_replace(char **str, char *start, size_t size,
        const char *token)
{
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

int variable_expand(char **str)
{
    size_t size;
    for (char *variable_ptr = strchr(*str, '$'); variable_ptr;
            variable_ptr = strchr(*str, '$'))
    {
        size = 0;
        if (variable_ptr[1] == '\n' || variable_ptr[1] == '\0')
            return 1;
        else if (variable_ptr[1] == '{' || variable_ptr[1] == '(')
        {
            if (variable_ptr[1] == '{')
            {
                size = strcspn(variable_ptr, " \t\r\n\v\f}:#=}") + 1;
                if (*(variable_ptr + size - 1) != '}')
                    return 0;
            }
            else
            {
                size = strcspn(variable_ptr, " \t\r\n\v\f):#=)") + 1;
                if (*(variable_ptr + size - 1) != ')')
                    return 0;
            }
            variable_replace(str, variable_ptr, size,
                    variable_get_value(variable_ptr + 2, size - 3));
        }
        else
        {
            variable_replace(str, variable_ptr, 1,
                    variable_get_value(variable_ptr + 1, 1));
        }
    }
    return 1;
}

void variable_free(void *variable_ptr)
{
    struct variable *variable = variable_ptr;
    free(variable->value);
    free(variable->name);
    free(variable);
}
