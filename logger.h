#include <stdio.h>
#ifndef LOGGER_H
#define LOGGER_H

#define LOG_DEBUG(...) logger_debug(__PRETTY_FUNCTION__, __VA_ARGS__)
#define LOG_INFO(...) logger_info(__PRETTY_FUNCTION__, __VA_ARGS__)
#define LOG_ERROR(...) logger_error(__PRETTY_FUNCTION__, __VA_ARGS__)
#define LOG_WARNING(...) logger_warning(__PRETTY_FUNCTION__, __VA_ARGS__)

void logger_debug(const char *func, const char *format, ...);
void logger_info(const char *func, const char *format, ...);
void logger_error(const char *func, const char *format, ...);
void logger_warning(const char *func, const char *format, ...);

#endif