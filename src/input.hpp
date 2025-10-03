/*
 * -------------------------------
 * Copyright (C) 2025 Connor Taylor.
 * Released under the MIT License.
 * -------------------------------
 *
 * Program name: books_and_brews.cpp
 * Author: Connor Taylor
 * Date last updated: 10/3/2025
 * Purpose: Define utility functions for reading input.
 */

#pragma once

#include <stdlib.h>
#include "logger.hpp"

#define EXIT -1
#define BAD_INPUT -2

void Flush();

struct range { int LowerBound, UpperBound; };
bool InRange(int Value, range Range);

void ClearScreen();
int ReadSelection(logger& Logger, range SelectionRange);

