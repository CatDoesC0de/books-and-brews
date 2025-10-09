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
#include "bb_common.hpp"
#include <stdio.h>
#include <string.h>

/*
 *  The funnel for showing every menu.
 *  This will handle the menu input, check for any invalid input
 *  then dispatch to the valid selection made by using the choice as an index
 *  into the Selections vector from menu.
 */
void ShowMenu(menu& Menu, logger& Logger, bool& IsRunning)
{
    ClearLogs(Logger);
    bool ShouldReprompt = true;
    while (ShouldReprompt && IsRunning)
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

menu CreateMenu(void (*PrintMenuFunction)(), u64 SelectionMinimum,
                std::vector<selection> Selections)
{
    menu Result = {};
    Result.Print = PrintMenuFunction;
    Result.SelectionRange = {SelectionMinimum, Selections.size()};
    Result.Selections = std::move(Selections);
    return Result;
}
