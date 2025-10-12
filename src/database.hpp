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

extern sqlite3* Database;

typedef const unsigned char Text;
struct statement_reader
{
    unsigned int ReadIndex;
    sqlite3_stmt* Statement;

    int integer();
    Text* text();
    double decimal();
};

bool DatabaseInit(const char* FileName);
void DatabaseClose();

bool Step(sqlite3_stmt* Cursor);
bool Prepare(sqlite3_stmt* Cursor);

int GetOutgoingOrderCount();

int GetItemCount();
sqlite3_stmt* GetItemList(); 
