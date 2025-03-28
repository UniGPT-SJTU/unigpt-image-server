#include <stdio.h>
#ifndef LOGGER_H
#define LOGGER_H

#define LOG_DEBUG(...) logger_debug(__PRETTY_FUNCTION__, __VA_ARGS__)
#define LOG_INFO(...) logger_info(__PRETTY_FUNCTION__, __VA_ARGS__)
#define LOG_ERROR(...) logger_error(__PRETTY_FUNCTION__, __VA_ARGS__)
#define LOG_WARNING(...) logger_warning(__PRETTY_FUNCTION__, __VA_ARGS__)

void logger_debug(char *func, char *format, ...);
void logger_info(char *func, char *format, ...);
void logger_error(char *func, char *format, ...);
void logger_warning(char *func, char *format, ...);

#endif