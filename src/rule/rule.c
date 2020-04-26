#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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
        rule = strchr(target, '%')
            ? linked_allocate(&g_parsed->pattern_rules, sizeof(struct rule))
            : linked_allocate(&g_parsed->rules, sizeof(struct rule));
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

static int rule_replace(struct error *err, const char *stem, char **dep)
{
    char *start = strchr(*dep, '%');
    if (!start)
        return 1;
    if (!variable_replace(dep, start, 1, stem))
        return exit_on_error(err, ERR_BAD_ALLOC, "*** allocation error.  Stop");
    return 1;
}

static struct rule *rule_copy_replace(struct error *err,
        const struct rule *rule, char *stem, const char *target)
{
    struct linked dependencies;
    struct linked commands;
    char *res_target = strdup(target);
    if (!res_target || !linked_str_copy(&commands, &rule->commands)
            || !linked_str_copy(&dependencies, &rule->dependencies)
            || !rule_assign(res_target, &dependencies, &commands))
    {
        free(res_target);
        exit_on_error(err, ERR_BAD_ALLOC, "*** allocation error.  Stop");
        return NULL;
    }
    for (struct _linked *l = dependencies.head; l; l = l->next)
    {
        if (!rule_replace(err, stem, (char **)&l->data))
            return NULL;
    }
    return rule_search(target);
}

static struct rule *rule_match(struct error *err, const char *target)
{
    struct rule *rule;
    char *rule_str;
    size_t rule_len;
    size_t rule_sep;
    size_t rule_remain;
    struct rule *rule_res = NULL;
    size_t rule_res_len = 0;
    char *stem = NULL;
    const size_t target_len = strlen(target);
    for (struct _linked *l = g_parsed->pattern_rules.head; l; l = l->next)
    {
        rule = l->data;
        rule_str = rule->target;
        rule_len = strlen(rule_str);
        rule_sep = strchr(rule_str, '%') - rule_str;
        rule_remain = rule_len - rule_sep - 1;
        if (rule_len > rule_res_len && target_len >= rule_len
                && !strncmp(target, rule->target, rule_sep)
                && !strcmp(target + target_len - rule_remain,
                    rule->target + rule_sep + 1))
        {
            rule_res = rule;
            rule_res_len = rule_len;
            stem = realloc(stem, target_len - rule_sep - rule_remain + 1);
            if (!stem)
            {
                exit_on_error(err, ERR_BAD_ALLOC, "*** allocation error.  "
                        "Stop");
                return NULL;
            }
            strncpy(stem, target + rule_sep,
                    target_len - rule_sep - rule_remain);
            stem[target_len - rule_sep - rule_remain] = '\0';
        }
    }
    if (rule_res)
        rule_res = rule_copy_replace(err, rule_res, stem, target);
    free(stem);
    return rule_res;
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

static int rule_exec(struct error *err, struct rule *rule)
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
                return exit_on_error(err, ERR_BAD_ALLOC,
                        "*** allocation error.  Stop");
            case 2:
                return exit_on_error(err, ERR_BAD_VAR,
                        "*** unterminated variable reference.  Stop");
            case 3:
                return exit_on_error(err, ERR_RECURSIVE_VAR,
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
        {
            char msg[ERROR_MESSAGE_SIZE];
            snprintf(msg, ERROR_MESSAGE_SIZE, "*** [Makefile: %s] "
                    "Error %d", rule->target, res);
            return exit_on_error(err, ERR_NO_RULE, msg);
        }
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

static int special_variables(struct error *err, const struct rule *rule)
{
    if (!variable_assign("@", rule->target))
    {
        return exit_on_error(err, ERR_BAD_ALLOC, "*** allocation error.  "
                "Stop");
    }
    if (!rule->dependencies.head)
    {
        if (!variable_assign("<", ""))
        {
            return exit_on_error(err, ERR_BAD_ALLOC, "*** allocation error.  "
                    "Stop");
        }
        if (!variable_assign("^", ""))
        {
            return exit_on_error(err, ERR_BAD_ALLOC, "*** allocation error.  "
                    "Stop");
        }
    }
    else
    {
        if (!variable_assign("<", rule->dependencies.head->data))
        {
            return exit_on_error(err, ERR_BAD_ALLOC, "*** allocation error.  "
                    "Stop");
        }
        char *to_add;
        char *deps = strdup(rule->dependencies.head->data);;
        if (!deps)
        {
            return exit_on_error(err, ERR_BAD_ALLOC, "*** allocation error.  "
                    "Stop");
        }
        size_t len = strlen(deps);
        size_t to_add_len;
        for (struct _linked *l = rule->dependencies.head->next; l;
                l = l->next, len += to_add_len + 1)
        {
            to_add = l->data;
            to_add_len = strlen(to_add);
            deps = realloc(deps, len + to_add_len + 2);
            if (!deps)
            {
                return exit_on_error(err, ERR_BAD_ALLOC, "*** allocation error.  "
                        "Stop");
            }
            deps[len] = ' ';
            strcpy(deps + len + 1, to_add);
        }
        if (!variable_assign("^", deps))
        {
            free(deps);
            return exit_on_error(err, ERR_BAD_ALLOC, "*** allocation error.  "
                    "Stop");
        }
        free(deps);
    }
    return 1;
}

static void exec_warn(const char *target, int phony_rule)
{
    if (phony_rule)
    {
        printf("%s: '%s' is up to date.\n",
                program_invocation_short_name, target);
    }
    else
    {
        printf("%s: Nothing to be done for '%s'.\n",
                program_invocation_short_name, target);
    }
}

static int _exec(struct error *err, const char *target, int top)
{
    int exec = 0;
    int built = 0;
    struct rule *rule;
    struct stat statbuf;
    struct timespec recent = { 0, 0 };
    rule = rule_search(target);
    if (!rule)
    {
        rule = rule_match(err, target);
        if (err->code)
            return 0;
        if (!rule)
        {
            if (top)
                errx(ERR_NO_RULE, "*** No rule to make target '%s'."
                        "  Stop.", target);
            return 0;
        }
    }
    int phony = is_phony(target);
    if (rule->is_built)
    {
        if (top)
            exec_warn(target, rule->commands.head && !phony);
        return 0;
    }
    for (struct _linked *dependencies = rule->dependencies.head;
            dependencies; dependencies = dependencies->next)
    {
        char *dep = dependencies->data;
        get_result(_exec(err, dep, 0), &built, &exec);
        if (err->code)
            return 0;
        if (!built)
        {
            if (stat(dep, &statbuf))
            {
                char msg[ERROR_MESSAGE_SIZE];
                snprintf(msg, ERROR_MESSAGE_SIZE, "*** No rule to make target "
                        "'%s', needed, by '%s'.  Stop.", dep, target);
                return exit_on_error(err, ERR_NO_RULE, msg);
            }
            timespec_max(&recent, &statbuf.st_mtim);
        }
    }
    if (built || stat(target, &statbuf) ||
            timespec_compare(&recent, &statbuf.st_mtim))
    {
        if (!special_variables(err, rule))
            return 0;
        exec = exec | (rule_exec(err, rule) ? EXEC : 0);
        if (err->code)
            return 0;
        if (!exec && top)
        {
            printf("%s: Nothing to be done for '%s'.\n",
                    program_invocation_short_name, target);
        }
        built = 1;
        return built | exec;
    }
    if (top)
        exec_warn(target, rule->commands.head && !phony);
    return built | exec;
}

void exec(char *targets[])
{
    struct error err;
    error_init(&err);
    if (!*targets)
    {
        struct rule *rule = NULL;
        struct rule *check;
        for (struct _linked *l = g_parsed->rules.head; l; l = l->next)
        {
            check = l->data;
            if (*check->target == '\0' || !strcmp(check->target, ".PHONY"))
                continue;
            rule = check;
            break;
        }
        if (!rule)
            errx(ERR_NO_TARGET, "*** No targets.  Stop.");
        _exec(&err, rule->target, 1);
        if (err.code)
            errx(err.code, err.msg);
        return;
    }
    for (size_t i = 0; targets[i]; ++i)
    {
        _exec(&err, targets[i], 1);
        if (err.code)
            errx(err.code, err.msg);
    }

}
