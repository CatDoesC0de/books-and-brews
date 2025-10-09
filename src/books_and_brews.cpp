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

#include "database.hpp"
#include "menu.hpp"
#include <stdio.h>

static sqlite3* Database;

static menu HomeMenu = {};
static menu AddMenu = {};
static selection Back = {"",
                         [](logger& Logger, bool& IsRunning) { return false; }};

static auto NotImplementedCallback = [](logger& Logger, bool& IsRunning) {
    BB_LOG_ERROR(Logger, "Not Implemented");
    return true;
};

int GetListSelection(sqlite3_stmt* List, int ListSize, const char* Prompt)
{
    while (true)
    {
        ClearScreen();
        PrintLogMessages(Logger);

        printf("Fetched %i rows. How many should be displayed per-page?\n",
               ListSize);

        int RowsPerPage;
        if (!ReadInt(Logger, RowsPerPage))
        {
            continue;
        }

        printf("%s\n", Prompt);
    }
}

bool AddOrderItemCallback(logger& Logger, bool& IsRunning)
{
    ClearScreen();
    PrintLogMessages(Logger);

    int ItemListCount = GetItemCount(Database);

    if (ItemListCount == 0)
    {
        BB_LOG_ERROR(Logger, "There must be menu items in the system to add an item to an order.");
        return true;
    }

    int OrderID;
    int OutgoingOrderCount = GetOutgoingOrderCount(Database);
    if (OutgoingOrderCount != 0)
    {
        printf("Use existing order? (Y/N)\n");

        char Choice;
        scanf("%c", &Choice);

        if (Choice != 'Y' && Choice != 'y' && Choice != 'N' && Choice != 'n')
        {
            BB_LOG_ERROR(Logger, "Answer must be Y or N");
            return true;
        }

        bool UseExisting = Choice == 'Y' || Choice == 'y';
        if (UseExisting)
        {}
    }

    sqlite3_stmt* ItemListStatement = GetItemList(Database);
    int ItemID = GetListSelection(ItemListStatement, ItemListCount,
                     "What item would you like to add to the menu?\n");
    return false;
}

menu CreateAddMenu()
{
    auto PrintFunction = []() {
        printf("What would you like to add? (0 to go back, -1 to exit)\n");
    };

    std::vector<selection> Selections;

    selection OrderItem = {};
    OrderItem.Label = "OrderItem";
    OrderItem.Handler = AddOrderItemCallback;

    Selections.push_back(Back);
    Selections.push_back(OrderItem);

    menu AddMenu = CreateMenu(PrintFunction, 0, Selections);
    return AddMenu;
}

menu CreateHomeMenu()
{
    auto PrintFunction = []() {
        printf("What would you like to do?. (-1 to exit)\n");
    };

    std::vector<selection> Selections;

    selection AddSelection = {};
    AddSelection.Label = "Add";
    AddSelection.Handler = [](logger& Logger, bool& IsRunning) {
        ShowMenu(AddMenu, Logger, IsRunning);
        return true;
    };

    Selections.push_back(AddSelection);

    menu HomeMenu = CreateMenu(PrintFunction, 1, Selections);
    return HomeMenu;
}

int main(int Argc, char** Argv)
{
    const char FileName[] = "books_and_brews.db";
    if (sqlite3_open_v2(FileName, &Database, SQLITE_OPEN_READWRITE, nullptr) !=
        SQLITE_OK)
    {
        fprintf(stderr, "Failed to open database with file '%s'.",
                sqlite3_errmsg(Database));
        return -1;
    }

    bool IsRunning = true;

    HomeMenu = CreateHomeMenu();
    AddMenu = CreateAddMenu();

    ShowMenu(HomeMenu, Logger, IsRunning);

    FreeLogger(Logger);
    return 0;
}
