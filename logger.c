#include "logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

static void logger_log(FILE *output, const char *type, const char *func, const char *format, va_list args);

#ifdef ENABLE_LOG
void logger_info(const char *func, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    logger_log(stdout, "INFO", func, format, args);
    fprintf(stdout, "\n");
    va_end(args);
}

void logger_error(const char *func, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "\033[1;31m");
    logger_log(stderr, "ERROR", func, format, args);
    fprintf(stderr, "\033[0m\n");
    va_end(args);
}

void logger_warning(const char *func, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    fprintf(stderr, "\033[1;35m");
    logger_log(stderr, "WARNING", func, format, args);
    fprintf(stderr, "\033[0m\n");
    va_end(args);
}
static void logger_log(FILE *output,const char *type, const char *func,  const char *format, va_list args) {
    time_t now = time(0);
    char *dt = ctime(&now);
    dt[strlen(dt) - 1] = '\0';
    fprintf(output, "[%s] %s: %s: ", dt, type, func);
    vfprintf(output, format, args);
}
#else 
void logger_info(const char *func, const char *format, ...)
{
}
void logger_error(const char *func, const char *format, ...)
{
}
void logger_warning(const char *func, const char *format, ...)
{
}
static void logger_log(FILE *output,  const char *type, const char *func, const char *format, va_list args)
{
}
#endif