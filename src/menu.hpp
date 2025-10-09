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
 *
 * NOTE(Connor): Very over-engineered for this application, but was fun to put together. 
 */

#pragma once

#include "input.hpp"
#include "logger.hpp"

#include <vector>

struct selection
{
    const char* Label;
    bool (*Handler)(logger& Logger, bool& IsRunning);
};

struct menu
{
    void (*Print)();
    std::vector<selection> Selections;
    range SelectionRange;
};

void ShowMenu(menu& Menu, logger& Logger, bool& IsRunning);
menu CreateMenu(void(*PrintMenuFunction)(), u64 SelectionMinimum, std::vector<selection> Selections);
void DeleteMenu(menu& Menu);
