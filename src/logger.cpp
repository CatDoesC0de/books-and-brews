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

#include "logger.hpp"
#include "bb_common.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BB_INFO(logger, Format, ...)                                           \
    Log(logger, FormatString(log_level::log_info, Format, ##__VA_ARGS__))
#define BB_WARN(logger, Format, ...)                                           \
    Log(logger, FormatString(log_level::log_warning, Format, ##__VA_ARGS__))
#define BB_ERROR(logger, Format, ...)                                          \
    Log(logger, FormatString(log_level::log_error, Format, ##__VA_ARGS__))

const char* LogLevelToString(log_level LogLevel)
{
    switch (LogLevel)
    {
        case log_level::log_info:
        {
            return "[INFO]:";
        }
        break;
        case log_level::log_warning:
        {
            return "[WARN]:";
        }
        break;
        case log_level::log_error:
        {
            return "[ERROR]:";
        }
        break;
        default:
        {
            return "[???]:";
        }
        break;
    }
}

char* FormatString(log_level LogLevel, const char* Format, ...)
{
    va_list VArgs;
    va_start(VArgs, Format);

    va_list VArgsCopy;
    va_copy(VArgsCopy, VArgs);
    int Size = vsnprintf(NULL, 0, Format, VArgsCopy);
    va_end(VArgsCopy);

    if (Size < 0)
    {
        va_end(VArgs);
        return NULL;
    }

    const char* Prefix = LogLevelToString(LogLevel);
    int PrefixLength = strlen(Prefix);

    char* String = (char*)malloc(PrefixLength + 1 + Size + 1);
    if (!String)
    {
        va_end(VArgs);
        return NULL;
    }

    strcpy(String, Prefix);
    strcat(String, " ");

    vsnprintf(String + PrefixLength + 1, Size + 1, Format, VArgs);
    va_end(VArgs);

    return String;
}

void Log(logger& Logger, const char* String)
{
    if (!Logger.Messages)
    {
        return;
    }

    char** Messages = Logger.Messages;
    char* LastMessage = Messages[Logger.MessagesSize - 1];
    free(LastMessage);

    for (int i = Logger.MessagesSize - 1; i > 0; i--)
    {
        Messages[i] = Messages[i - 1];
    }

    Messages[0] = (char*) String;
}

void PrintLogMessages(logger& Logger)
{
    printf("\t\t[%s]\n", Logger.Name);
    for (int i = Logger.MessagesSize - 1; i >= 0; i--)
    {
        char* Message = Logger.Messages[i];

        if (Message)
        {
            printf("%s\n", Message);
        }
    }
}

logger CreateLogger(const char* Name, u32 Messages)
{
    logger Logger = {};
    Logger.Messages = (char**)malloc(sizeof(char*) * Messages);
    Logger.Name = Name;

    for (int i = 0; i < Messages; i++)
    {
        Logger.Messages[i] = nullptr;
    }

    Logger.MessagesSize = Messages;
    return Logger;
}

void FreeLogger(logger& Logger)
{
    ClearLogs(Logger);
    free(Logger.Messages);
    Logger = {};
}

void ClearLogs(logger& Logger)
{
    char** Messages = Logger.Messages;
    if (Messages)
    {
        for (int i = 0; i < Logger.MessagesSize; i++)
        {
            free(Messages[i]);
            Messages[i] = nullptr;
        }
    }
}
