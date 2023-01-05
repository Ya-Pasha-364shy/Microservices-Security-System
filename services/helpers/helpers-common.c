#include "../../include/helpers/helpers-common.h"

void strinit(char * string)
{
    string[0] = '\0';
}

void message_init(void * message, size_t * message_size)
{
    message = malloc(*message_size * sizeof(void));
    memset(message, 0, *message_size);
}