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
#include <regex>
#include <string>

void ClearScreen();

bool ReadInt(int& Result);
bool ReadPositiveFloat(float& Result);
bool ReadPositiveInt(int& Result);
bool ReadBool(bool& Result);
bool ReadString(std::string& Result, std::regex Pattern);
