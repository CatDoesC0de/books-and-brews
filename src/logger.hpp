/*
 * -------------------------------
 * Copyright (C) 2025 Connor Taylor.
 * Released under the MIT License.
 * -------------------------------
 *
 * Program name: books_and_brews.cpp
 * Author: Connor Taylor
 * Date last updated: 10/3/2025
 * Purpose: Define a logger and provide utililty functions
 */
#pragma once

#define BB_LOG_INFO(Format, ...)                                           \
    Log(FormatString(log_level::log_info, Format, ##__VA_ARGS__))
#define BB_LOG_WARN(Format, ...)                                           \
    Log(FormatString(log_level::log_warning, Format, ##__VA_ARGS__))
#define BB_LOG_ERROR(Format, ...)                                          \
    Log(FormatString(log_level::log_error, Format, ##__VA_ARGS__))

#ifdef BB_DEBUG_BUILD
    #define BB_LOG_DEBUG(Format, ...)                                          \
        Log(FormatString(log_level::log_debug, Format, ##__VA_ARGS__))
#else
    #define BB_LOG_DEBUG(Format, ...)
#endif

enum class log_level
{
    log_error,
    log_info,
    log_warning,
    log_debug
};

struct logger
{
    const char* Name;
    char** Messages;
    unsigned int MessagesSize;
};

extern logger S_Logger;
 
//Initialize the global S_Logger, called only once and before any logging.
void LoggerInit(logger Logger); 

logger CreateLogger(const char* Name, unsigned int Messages);
void FreeLogger();
void ClearLogs();
void Log(char* String);
void PrintLogs();
char* FormatString(log_level LogLevel, const char* Format, ...);
