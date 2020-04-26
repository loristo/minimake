#ifndef ERROR_H
#define ERROR_H

#define ERROR_MESSAGE_SIZE 64

struct error
{
    int code;
    char msg[ERROR_MESSAGE_SIZE];
};

int exit_on_error(struct error *error, int status, const char *str);
void error_init(struct error *error);

#endif /* ERROR_H */
