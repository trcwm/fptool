/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  logging system

  Author: Niels A. Moseley

*/

#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include "logging.h"

static bool g_debugEnabled = false;
static FILE* g_logFile = NULL;

void setDebugging(bool enabled)
{
    g_debugEnabled = enabled;
}

bool setLogFile(const char *filename)
{
    if (g_logFile != NULL)
    {
        fclose(g_logFile);
    }
    g_logFile = fopen(filename, "wt");
    return (g_logFile != NULL);
}

void closeLogFile()
{
    if (g_logFile != NULL)
    {
        fclose(g_logFile);
        g_logFile = NULL;
    }
}

void doLog(logtype_t t, const char *format, ...)
{
    switch(t)
    {
    case LOG_INFO:
        std::cout << "INFO: ";
        if (g_logFile != NULL) fprintf(g_logFile, "INFO: ");
        break;
    case LOG_DEBUG:
        if (!g_debugEnabled) return;
        std::cout << "DEBUG: ";
        if (g_logFile != NULL) fprintf(g_logFile, "DEBUG: ");
        break;
    case LOG_WARN:
        std::cout << "WARNING: ";
        if (g_logFile != NULL) fprintf(g_logFile, "WARNING: ");
        break;
    case LOG_ERROR:
        std::cout << "ERROR: ";
        if (g_logFile != NULL) fprintf(g_logFile, "ERROR: ");
        break;
    default:
        break;
    }

    //FIXME: change to C++ style
    va_list argptr;
    va_start(argptr, format);
    vprintf(format, argptr);
    if (g_logFile != NULL)
    {
        vfprintf(g_logFile, format, argptr);
    }
    va_end(argptr);
}
