#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

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
    rule->is_built = 0;
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

static int rule_exec(struct rule *rule)
{
    rule->is_built = 1;
    return 1;
}

static long max(long a, long b)
{
    return a > b ? a : b;
}

static int _exec(char *targets[], int top)
{
    int built = 0;
    struct rule *rule;
    struct stat statbuf;
    long recent = 0;
    for (size_t i = 0; targets[i]; ++i)
    {
        rule = rule_search(targets[i]);
        if (!rule)
        {
            if (top)
                errx(ERR_NO_RULE, "*** No rule to make target '%s'."
                        "  Stop.", targets[i]);
            return 0;
        }
        if (rule->is_built)
        {
            if (top)
                printf("minimake: '%s' is up to date.\n", targets[i]);
            continue;
        }
        for (struct _linked *dependencies = rule->dependencies.head;
                dependencies; dependencies = dependencies->next)
        {
            char *dep = dependencies->data;
            char *dependencies_target[] = { dep, NULL };
            int res = _exec(dependencies_target, 0);
            if (res)
                built = 1;
            else
            {
                if (stat(dep, &statbuf))
                    errx(ERR_NO_RULE, "*** No rule to make target '%s', "
                            "needed, by '%s'.  Stop.", dep, targets[i]);
                recent = max(recent, statbuf.st_mtim.tv_nsec);
            }
        }
        if (built || stat(targets[i], &statbuf) || recent > statbuf.st_mtim.tv_nsec)
        {
            if (!rule_exec(rule) && top)
                printf("minimake: Nothing to be done for '%s'.\n", targets[i]);
            continue;
        }
        if (top)
                printf("minimake: '%s' is up to date.\n", targets[i]);
    }
    return 1;
}

void exec(char *targets[])
{
    if (!*targets)
    {
        struct rule *rule = g_parsed->rules.head->data;
        char *new[] = { rule->target, NULL };
        rule_exec(rule);
        _exec(new, 1);
        return;
    }
    _exec(targets, 1);
}
