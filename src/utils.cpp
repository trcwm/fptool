
#include <stdarg.h>
#include "utils.h"

std::string vstringf(const char *fmt, va_list ap)
{
    std::string string;
    char *str = NULL;

#ifdef _WIN32
    int sz = 64, rc;
    while (1) {
        va_list apc;
        va_copy(apc, ap);
        str = (char*)realloc(str, sz);
        rc = vsnprintf(str, sz, fmt, apc);
        va_end(apc);
        if (rc >= 0 && rc < sz)
            break;
        sz *= 2;
    }
#else
    if (vasprintf(&str, fmt, ap) < 0)
        str = NULL;
#endif

    if (str != NULL) {
        string = str;
        free(str);
    }

    return string;
}

std::string stringf(const char *fmt, ...)
{
    std::string string;
    va_list ap;

    va_start(ap, fmt);
    string = vstringf(fmt, ap);
    va_end(ap);

    return string;
}
