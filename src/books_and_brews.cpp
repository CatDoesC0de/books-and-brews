/*
 * -------------------------------
 * Copyright (C) 2025 Connor Taylor.
 * Released under the MIT License.
 * -------------------------------
 *
 * Program name: books_and_brews.cpp
 * Author: Connor Taylor
 * Date last updated: 10/2/2025
 * Purpose: Create a software interface for managing the Books and Brews
 * inventory.
 */

#include "bb_common.hpp"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT -1
#define BAD_INPUT -2

#define BB_INFO(logger, Format, ...)                                           \
    Log(logger, FormatString(log_level::log_info, Format, ##__VA_ARGS__))
#define BB_WARN(logger, Format, ...)                                           \
    Log(logger, FormatString(log_level::log_warning, Format, ##__VA_ARGS__))
#define BB_ERROR(logger, Format, ...)                                          \
    Log(logger, FormatString(log_level::log_error, Format, ##__VA_ARGS__))

enum class log_level
{
    log_error,
    log_info,
    log_warning
};

struct logger
{
    char** Messages;
    u32 MessagesSize;
};

logger CreateLogger(u32 Messages)
{
    logger Logger = {};
    Logger.Messages = (char**)malloc(sizeof(char*) * Messages);
    for (int i = 0; i < Messages; i++)
    {
        Logger.Messages[i] = nullptr;
    }

    Logger.MessagesSize = Messages;
    return Logger;
}

void FreeLogger(logger& Logger)
{
    char** Messages = Logger.Messages;
    if (Messages)
    {
        for (int i = 0; i < Logger.MessagesSize; i++)
        {
            free(Messages[i]);
        }
        free(Messages);
    }

    Logger = {};
}

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

void Log(logger& Logger, char* String)
{
    char** Messages = Logger.Messages;
    char* LastMessage = Messages[Logger.MessagesSize - 1];
    free(LastMessage);

    for (int i = Logger.MessagesSize - 1; i > 0; i--)
    {
        Messages[i] = Messages[i - 1];
    }

    Messages[0] = String;
}

void PrintLogMessages(logger& Logger)
{
    printf("\t\t[Logs]\n");
    for (int i = Logger.MessagesSize - 1; i >= 0; i--)
    {
        char* Message = Logger.Messages[i];

        if (Message)
        {
            printf("%s\n", Message);
        }
    }
}

void Flush()
{
    int Character;
    while ((Character = getchar()) != '\n' && Character != EOF)
        ;
}

bool InRange(int Value, int LowerBound, int UpperBound)
{
    bool Result = Value >= LowerBound && Value <= UpperBound;
    return Result;
}

void ClearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int ReadSelection(logger& Logger, int LowerBound, int UpperBound)
{
    int Selection = BAD_INPUT;
    int Error = scanf("%d", &Selection);
    Flush();

    if (Error == EOF || Error == 0)
    {
        BB_ERROR(Logger, "Invalid Selection. Expected numeric input.");
        Selection = BAD_INPUT;
    }
    else if (Selection == EXIT)
    {
        Selection = EXIT;
    }
    else if (!InRange(Selection, LowerBound, UpperBound))
    {
        Selection = BAD_INPUT;
        BB_ERROR(Logger, "Invalid Selection. Expected value in range [%d, %d]",
               LowerBound, UpperBound);
    }

    return Selection;
}

int main(int Argc, char** Argv)
{
    ClearScreen();
    logger Logger = CreateLogger(10);

    PrintLogMessages(Logger);
    int Choice;
    do
    {
        ClearScreen();
        PrintLogMessages(Logger);

        printf("\n\n\n\nBooks and Brews Inventory Tracking System:\n");
        printf("\n");
        printf("1) %s\n", "Create");

        Choice = ReadSelection(Logger, 1, 1);

        if (Choice != BAD_INPUT)
        {
            break;
        }

    } while (true);

    FreeLogger(Logger);
    return 0;
}
