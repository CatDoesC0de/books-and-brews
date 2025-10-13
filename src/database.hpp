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
    statement_reader(sqlite3_stmt* Statement);
    int integer();
    Text* text();
    double decimal();

    private:
    unsigned int ReadIndex;
    sqlite3_stmt* Statement;
};

struct statement_binder
{
    statement_binder(sqlite3_stmt* Statement);
    void integer(int Value);

    private:
    unsigned int BindIndex;
    sqlite3_stmt* Statement;
};

bool DatabaseInit(const char* FileName);
void DatabaseClose();

void Transaction();
void Commit();
void Rollback();

bool Step(sqlite3_stmt* Cursor);
bool Prepare(sqlite3_stmt* Cursor);

bool CreateOrder(int& Result);
bool AddItemToOrder(int OrderNumber, int ItemID, int ItemQuantity);
int GetOutgoingOrderCount();

sqlite3_stmt* GetItem(int ItemID);
int GetItemCount();
sqlite3_stmt* GetItemList(); 
