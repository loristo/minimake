#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>

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

int variable_assign(const char *var_name, const char *var_value)
{
    struct variable *variable = variable_search(var_name);
    if (!variable)
    {
        variable = linked_allocate(&g_parsed->variables,
                sizeof(struct variable));
        if (!variable)
                return 0;
    }
    variable->name = strdup(var_name);
    variable->value = strdup(var_value);
    if (!variable->value || !variable->value)
    {
        free(variable->value);
        free(variable->name);
        free(variable);
        return 0;
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
