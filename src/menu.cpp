/*
 * -------------------------------
 * Copyright (C) 2025 Connor Taylor.
 * Released under the MIT License.
 * -------------------------------
 *
 * Program name: books_and_brews.cpp
 * Author: Connor Taylor
 * Date last updated: 10/2/2025
 * Purpose: Define utility functions to define menus and their selections
 */

#include "menu.hpp"
#include <stdio.h>
#include <string.h>

void ShowMenu(menu& Menu, logger& Logger, bool& IsRunning)
{
    ClearLogs(Logger);
    bool ShouldReprompt = true;
    while (ShouldReprompt)
    {
        ClearScreen();
        PrintLogMessages(Logger);

        Menu.Print();

        int LabelCounter = 1;
        for (int i = 0; i < Menu.Selections.size(); i++)
        {
            selection Selection = Menu.Selections[i];

            if (strcmp(Selection.Label, "") != 0)
            {
                printf("%d) %s\n", LabelCounter++, Selection.Label);
            }
        }

        int Selection = ReadSelection(Logger, Menu.SelectionRange);

        if (Selection == BAD_INPUT)
        {}
        // Don't catch -1 if its included in the range
        else if (Selection == EXIT && !InRange(Selection, Menu.SelectionRange))
        {
            IsRunning = false;
            ShouldReprompt = false;
        }
        else if (!Menu.Selections.empty())
        {
            int SelectionIndex =
                Menu.SelectionRange.LowerBound == 0 ? Selection : Selection - 1;
            ShouldReprompt =
                Menu.Selections[SelectionIndex].Handler(Logger, IsRunning);
        }
    }
}

menu CreateMenu(void (*PrintMenuFunction)(), range SelectionRange)
{
    menu Result = {};
    Result.Print = PrintMenuFunction;
    Result.SelectionRange = SelectionRange;
    return Result;
}
