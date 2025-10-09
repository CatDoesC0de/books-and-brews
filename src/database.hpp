/*
 * -------------------------------
 * Copyright (C) 2025 Connor Taylor.
 * Released under the MIT License.
 * -------------------------------
 *
 * Program name: books_and_brews.cpp
 * Author: Connor Taylor
 * Date last updated: 10/9/2025
 * Purpose: Define utility functions for interacting with the database
 */

#pragma once

#include "sqlite3.h"

bool Step(sqlite3_stmt* Cursor, sqlite3* Database);
bool Prepare(sqlite3_stmt* Cursor, sqlite3* Database);

int GetOutgoingOrderCount(sqlite3* Database);

int GetItemCount(sqlite3* Database);
sqlite3_stmt* GetItemList(sqlite3* Database); 
