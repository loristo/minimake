#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>

#include <minimake.h>
#include <rule/rule.h>

static struct rule *rule_search(const char *target)
{
    struct rule *rule;
    for (struct _linked *l = g_parsed->rules.head; l; l = l->next)
    {
        rule = l->data;
        if (!strcmp(target, rule->target))
            return rule;
    }
    return NULL;
}

int rule_assign(char *target, struct linked *dependencies,
        struct linked *commands)
{
    struct rule *rule = rule_search(target);
    if (!rule)
    {
        rule = linked_allocate(&g_parsed->rules,
                sizeof(struct rule));
        if (!rule)
        {
            linked_free(commands, NULL);
            linked_free(dependencies, NULL);
            return 0;
        }
    }
    else
    {
        free(&rule->target);
        linked_free(&rule->dependencies, NULL);
        linked_free(&rule->commands, NULL);
    }
    rule->target = target;
    rule->commands.head = commands->head;
    rule->commands.tail = commands->tail;
    rule->dependencies.head = dependencies->head;
    rule->dependencies.tail = dependencies->tail;
    return 1;
}

void rule_free(void *rule_ptr)
{
    struct rule *rule = rule_ptr;
    free(rule->target);
    linked_free(&rule->commands, NULL);
    linked_free(&rule->dependencies, NULL);
    free(rule);
}
