/*

  FPTOOL - a fixed-point math to VHDL generation tool

  Description:  logging system

  Author: Niels A. Moseley

*/

#ifndef logging_h
#define logging_h

#include <string>

typedef enum {LOG_INFO = 1, LOG_DEBUG = 2, LOG_WARN = 4, LOG_ERROR = 8} logtype_t;

/** enable the debug output */
void setDebugging(bool enabled = true);

/** set log filename to log to a file.
    returns true if successful. */
bool setLogFile(const char *filename);

/** close the log file */
void closeLogFile();

/** log something */
void doLog(logtype_t t, const char *format, ...);

#endif
