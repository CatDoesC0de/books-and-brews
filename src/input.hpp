/*
 * -------------------------------
 * Copyright (C) 2025 Connor Taylor.
 * Released under the MIT License.
 * -------------------------------
 *
 * Program name: input.hpp
 * Author: Connor Taylor
 * Last Update: 10/16/2025
 * Purpose: Define utility functions for reading input.
 */

#pragma once

#include <stdlib.h>
#include <regex>
#include <string>

void ClearScreen();

bool ReadInt(int& Result);
bool ReadPositiveDouble(double& Result);
bool ReadPositiveInt(int& Result);
bool ReadBool(bool& Result);
bool ReadString(std::string& Result, std::regex Pattern);
