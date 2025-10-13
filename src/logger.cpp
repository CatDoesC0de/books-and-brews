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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* LogLevelToString(log_level LogLevel)
{
    switch (LogLevel)
    {
        case log_level::log_info: return "[INFO]:"; break;
        case log_level::log_warning: return "[WARN]:"; break;
        case log_level::log_error: return "[ERROR]:"; break;
        case log_level::log_debug: return "[DEBUG]:"; break;
        default: return "[???]:"; break;
    }
}

logger S_Logger;

void LoggerInit(logger Logger)
{
    S_Logger = Logger;
}

logger CreateLogger(const char* Name, unsigned int Messages)
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

void Log(char* String)
{
    if (!S_Logger.Messages)
    {
        return;
    }

    char** Messages = S_Logger.Messages;
    char* LastMessage = Messages[S_Logger.MessagesSize - 1];
    free(LastMessage);

    for (int i = S_Logger.MessagesSize - 1; i > 0; i--)
    {
        Messages[i] = Messages[i - 1];
    }

    Messages[0] = (char*)String;
}

    void PrintLogs()
{
    printf("\t\t[%s]\n", S_Logger.Name);
    for (int i = S_Logger.MessagesSize - 1; i >= 0; i--)
    {
        char* Message = S_Logger.Messages[i];

        if (Message)
        {
            printf("%s\n", Message);
        }
    }
    printf("\n");
}

void FreeLogger()
{
    ClearLogs();
    free(S_Logger.Messages);
    S_Logger = {};
}

void ClearLogs()
{
    char** Messages = S_Logger.Messages;
    if (Messages)
    {
        for (int i = 0; i < S_Logger.MessagesSize; i++)
        {
            free(Messages[i]);
            Messages[i] = nullptr;
        }
    }
}
