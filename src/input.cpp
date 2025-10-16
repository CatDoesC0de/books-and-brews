/* -------------------------------
 * Copyright (C) 2025 Connor Taylor.
 * Released under the MIT License.
 * -------------------------------
 *
 * Program name: input.cpp
 * Author: Connor Taylor
 * Last Update: 10/16/2025
 * Purpose: Define utility functions for reading input.
 */

#include "input.hpp"
#include "logger.hpp"
#include <iostream>
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

bool ReadPositiveInt(int& Result)
{
    int Input;
    if (ReadInt(Input))
    {
        if (Input <= 0)
        {
            return false;
        }
    }

    Result = Input; 
    return true;
}

bool ReadPositiveDouble(double& Result)
{
    double ReadResult;
    int Error = scanf("%lf", &ReadResult);
    Flush();

    if (Error == EOF || Error == 0)
    {
        BB_LOG_ERROR("Invalid Selection. Expected decimal input.");
        return false;
    }

    if (ReadResult <= 0)
    {
        return false;
    }

    Result = ReadResult;
    return true;
}

bool ReadBool(bool& Result)
{
    char ReadResult;
    int Error = scanf("%c", &ReadResult);
    Flush();

    if (Error == EOF || Error == 0)
    {
        BB_LOG_ERROR("Invalid Selection. Expected character.");
        return false;
    }

    if (ReadResult == 'Y' || ReadResult == 'y' || ReadResult == 'N' ||
        ReadResult == 'n')
    {
        Result = (ReadResult == 'Y' || ReadResult == 'y');
    }
    else
    {
        BB_LOG_ERROR("Invalid selection. Expected Y/y or N/n");
        return false;
    }

    return true;
}

bool ReadString(std::string& Result, std::regex Pattern)
{
    std::string Input;
    std::getline(std::cin, Input);

    if (!std::regex_match(Input, Pattern))
    {
        return false;
    }

    Input.erase(0, Input.find_first_not_of(' '));
    Input.erase(Input.find_last_not_of(' ') + 1);

    Result = std::move(Input);
    return true;
}
