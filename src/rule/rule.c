#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>
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
        free(rule->target);
        linked_free(&rule->dependencies, NULL);
        linked_free(&rule->commands, NULL);
    }
    rule->target = target;
    rule->commands.head = commands->head;
    rule->commands.tail = commands->tail;
    rule->dependencies.head = dependencies->head;
    rule->dependencies.tail = dependencies->tail;
    rule->is_built = 0;
    if (!strcmp(target, ".PHONY"))
        g_parsed->phony = &rule->dependencies;
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

static int command_exec(void **cmd)
{
    int status;
    int pid = fork();
    if (pid == -1)
        return -1;
    if (pid)
        waitpid(pid, &status, 0);
    else
    {
        char *args[] = { "/bin/sh", "-c", *cmd, NULL };
        if (*args[2] == '@')
            *args[2] = ' ';
        *cmd = NULL;
        if (execvp(args[0], args))
        {
            free(args[2]);
            err(-1, "execve failed");
        }
    }
    return status;
}

static int rule_exec(struct rule *rule)
{
    rule->is_built = 1;
    int res = 0;
    char **str;
    for (struct _linked *command = rule->commands.head; command;
            command = command->next)
    {
        res = 1;
        str = (char **)&command->data;
        int res = variable_expand(str, 1);
        switch (res)
        {
            case 0:
                break;
            case 1:
                errx(ERR_BAD_ALLOC, "*** allocation error.  Stop");
            case 2:
                errx(ERR_BAD_VAR, "*** unterminated variable reference.  Stop");
            case 3:
                errx(ERR_RECURSIVE_VAR,
                    "*** Recursive variable references itself (eventually)."
                    "  Stop.");
            default:
                break;
        }
        if (**str != '@')
            puts(*str);
        fflush(stdout);
        res = command_exec(&command->data);
        if (res)
            errx(ERR_EXEC, "*** [Makefile: %s] Error %d", rule->target, res);
    }
    return res;
}

static int timespec_compare(struct timespec *a, struct timespec *b)
{
    if (a->tv_sec < b->tv_sec)
        return 0;
    if (a->tv_sec == b->tv_sec)
        return a->tv_nsec > b->tv_nsec;
    return 1;
}

static void timespec_max(struct timespec *a, struct timespec *b)
{
    if (!timespec_compare(a, b))
        memcpy(a, b, sizeof(struct timespec));
}

static void get_result(int return_code, int *built, int *exec)
{
    *built = (return_code & BUILT) | *built;
    *exec = (return_code & EXEC) | *exec;
}

static int is_phony(const char *target)
{
    if (g_parsed->phony)
    {
        char *str;
        for (struct _linked *l = g_parsed->phony->head; l; l = l->next)
        {
            str = l->data;
            if (!strcmp(target, str))
                return 1;
        }
    }
    return 0;
}

static void special_variables(const struct rule *rule)
{
    if (!variable_assign("@", rule->target))
        err(ERR_BAD_ALLOC, "*** allocation error.  Stop");
    if (!rule->dependencies.head)
    {
        if (!variable_assign("<", ""))
            err(ERR_BAD_ALLOC, "*** allocation error.  Stop");
        if (!variable_assign("^", ""))
            err(ERR_BAD_ALLOC, "*** allocation error.  Stop");
    }
    else
    {
        if (!variable_assign("<", rule->dependencies.head->data))
            err(ERR_BAD_ALLOC, "*** allocation error.  Stop");
        char *to_add;
        char *deps = strdup(rule->dependencies.head->data);;
        if (!deps)
            err(ERR_BAD_ALLOC, "*** allocation error.  Stop");
        size_t len = strlen(deps);
        size_t to_add_len;
        for (struct _linked *l = rule->dependencies.head->next; l;
                l = l->next, len += to_add_len + 1)
        {
            to_add = l->data;
            to_add_len = strlen(to_add);
            deps = realloc(deps, len + to_add_len + 2);
            if (!deps)
                err(ERR_BAD_ALLOC, "*** allocation error.  Stop");
            deps[len] = ' ';
            strcpy(deps + len + 1, to_add);
        }
        if (!variable_assign("^", deps))
        {
            free(deps);
            err(ERR_BAD_ALLOC, "*** allocation error.  Stop");
        }
        free(deps);
    }
}

static int _exec(const char *target, int top)
{
    int exec = 0;
    int built = 0;
    struct rule *rule;
    struct stat statbuf;
    struct timespec recent = { 0, 0 };
    rule = rule_search(target);
    if (!rule)
    {
        if (top)
            errx(ERR_NO_RULE, "*** No rule to make target '%s'."
                    "  Stop.", target);
        return 0;
    }
    int phony = is_phony(target);
    if (rule->is_built)
    {
        if (top)
        {
            rule->commands.head && !phony
                ? printf("minimake: '%s' is up to date.\n", target)
                : printf("minimake: Nothing to be done for '%s'.\n", target);
        }
       return 0;
    }
    for (struct _linked *dependencies = rule->dependencies.head;
            dependencies; dependencies = dependencies->next)
    {
        char *dep = dependencies->data;
        get_result(_exec(dep, 0), &built, &exec);
        if (!built)
        {
            if (stat(dep, &statbuf))
                errx(ERR_NO_RULE, "*** No rule to make target '%s', "
                        "needed, by '%s'.  Stop.", dep, target);
            timespec_max(&recent, &statbuf.st_mtim);
        }
    }
    if (built || stat(target, &statbuf) ||
            timespec_compare(&recent, &statbuf.st_mtim))
    {
        special_variables(rule);
        exec = exec | (rule_exec(rule) ? EXEC : 0);
        if (!exec && top)
            printf("minimake: Nothing to be done for '%s'.\n", target);
        built = 1;
        return built | exec;
    }
    if (top)
    {
        rule->commands.head && !phony
            ? printf("minimake: '%s' is up to date.\n", target)
            : printf("minimake: Nothing to be done for '%s'.\n", target);
    }
    return built | exec;
}

void exec(char *targets[])
{
    if (!*targets)
    {
        struct rule *rule = NULL;
        struct rule *check;
        for (struct _linked *l = g_parsed->rules.head; l; l = l->next)
        {
            check = l->data;
            if (*check->target == '\0')
                continue;
            rule = check;
            break;
        }
        if (!rule)
            errx(ERR_NO_TARGET, "*** No targets.  Stop.");
        _exec(rule->target, 1);
        return;
    }
    for (size_t i = 0; targets[i]; ++i)
        _exec(targets[i], 1);
}
