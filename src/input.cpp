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
#include "logger.hpp"

#include <stdio.h>
#include <stdlib.h>

static void Flush()
{
    int Character;
    while ((Character = getchar()) != '\n' && Character != EOF)
        ;
}

void ClearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

bool ReadInt(int& Result)
{
    int ReadResult;
    int Error = scanf("%i", &ReadResult);
    Flush();

    if (Error == EOF || Error == 0)
    {
        BB_LOG_ERROR("Invalid Selection. Expected numeric input.");
        return false;
    }

    Result = ReadResult;
    return true;
}
