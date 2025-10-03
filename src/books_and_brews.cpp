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

#include "menu.hpp"
#include <stdio.h>

static menu HomeMenu = {};
static menu AddMenu = {};
static selection Back = {"",
                         [](logger& Logger, bool& IsRunning) { return false; }};

static auto NotImplementedCallback = [](logger& Logger, bool& IsRunning) {
    BB_ERROR(Logger, "Not Implemented");
    return true;
};


menu CreateAddMenu()
{
    auto PrintFunction = []() {
        printf("What would you like to add? (0 to go back, -1 to exit)\n");
    };

    range SelectionRange = {0, 2};

    selection AddIngredient = {};
    AddIngredient.Label = "Ingredient";
    AddIngredient.Handler = NotImplementedCallback;

    selection AddItem = {};
    AddItem.Label = "Item";
    AddItem.Handler = NotImplementedCallback;

    menu AddMenu = CreateMenu(PrintFunction, SelectionRange);

    AddMenu.Selections.push_back(Back);
    AddMenu.Selections.push_back(AddIngredient);
    AddMenu.Selections.push_back(AddItem);

    return AddMenu;
}

menu CreateHomeMenu()
{
    auto PrintFunction = []() {
        printf("What would you like to do?. (-1 to exit)\n");
    };

    range SelectionRange = {1, 1};
    menu HomeMenu = CreateMenu(PrintFunction, SelectionRange);

    selection AddSelection = {};
    AddSelection.Label = "Add";
    AddSelection.Handler = [](logger& Logger, bool& IsRunning) {
        ShowMenu(AddMenu, Logger, IsRunning);
        return true;
    };

    HomeMenu.Selections.push_back(AddSelection);

    return HomeMenu;
}

int main(int Argc, char** Argv)
{
    logger Logger = CreateLogger("Logs", 10);
    bool IsRunning = true;

    HomeMenu = CreateHomeMenu();
    AddMenu = CreateAddMenu();

    ShowMenu(HomeMenu, Logger, IsRunning);

    FreeLogger(Logger);
    return 0;
}
