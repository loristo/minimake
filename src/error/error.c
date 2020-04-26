#include <string.h>

#include <error/error.h>

int exit_on_error(struct error *error, int status, const char *str)
{
    error->code = status;
    strncpy(error->msg, str, ERROR_MESSAGE_SIZE);
    error->msg[ERROR_MESSAGE_SIZE - 1] = '\0';
    return 0;
}

void error_init(struct error *error)
{
    error->msg[0] = '\0';
    error->code = 0;
}
