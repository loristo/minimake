#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int status;
    pid_t pid = fork();
    if (pid == -1)
        return -1;
    else if (!pid)
    {
        char * const args[] = {"/bin/sh", "-c", "echo coucou", NULL};
        if (execve(args[0], args, NULL) == -1)
            return -1;
    }
    else
    {
        waitpid(pid, &status, 0);
        fflush(stdout);
    }
    return status;
}
