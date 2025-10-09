/* -------------------------------
 * Copyright (C) 2025 Connor Taylor.
 * Released under the MIT License.
 * -------------------------------
 *
 * Program name: books_and_brews.cpp
 * Author: Connor Taylor
 * Date last updated: 10/3/2025
 * Purpose: Define utility functions for reading input.
 */

#include "input.hpp"

#include <stdio.h>
#include <stdlib.h>

#define EXIT -1
#define BAD_INPUT -2

void Flush()
{
    int Character;
    while ((Character = getchar()) != '\n' && Character != EOF)
        ;
}

bool InRange(int Value, range Range)
{
    bool Result = Value >= Range.LowerBound && Value <= Range.UpperBound;
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

int ReadSelection(logger& Logger, range SelectionRange)
{
    int Selection = BAD_INPUT;
    printf(">> ");
    int Error = scanf("%d", &Selection);
    Flush();

    if (Error == EOF || Error == 0)
    {
        BB_LOG_ERROR(Logger, "Invalid Selection. Expected numeric input.");
        Selection = BAD_INPUT;
    }
    else if (Selection == EXIT)
    {
        Selection = EXIT;
    }
    else if (!InRange(Selection, SelectionRange))
    {
        Selection = BAD_INPUT;
        BB_LOG_ERROR(Logger, "Invalid Selection. Expected value in range [%d, %d]",
                 SelectionRange.LowerBound, SelectionRange.UpperBound);
    }

    return Selection;
}

bool ReadInt(logger& Logger, int& Result)
{
    int ReadResult;
    int Error = scanf("%d", &ReadResult);
    Flush();

    if (Error == EOF || Error == 0)
    {
        BB_LOG_ERROR(Logger, "Invalid Selection. Expected numeric input.");
        return false;
    }

    Result = ReadResult;
    return true;
}
