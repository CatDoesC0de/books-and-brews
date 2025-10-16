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
#include <cstdint>

extern sqlite3* Database;

typedef const char Text;
struct row_reader
{
    row_reader(sqlite3_stmt* Statement);
    row_reader(sqlite3_stmt* Statement, int ReadIndex);
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
    statement_binder& integer(int Value);

    private:
    unsigned int BindIndex;
    sqlite3_stmt* Statement;
};

bool DatabaseInit(const char* FileName);
void DatabaseClose();

void Transaction();
void Commit();
void Rollback();

int64_t LastInsertRowID();

bool StepRow(sqlite3_stmt* Cursor);
bool Prepare(sqlite3_stmt* Cursor);

bool CreateOrder(int64_t& Result);
bool AddItemToOrder(int OrderNumber, int ItemID, int ItemQuantity);
int GetOrderCount();
sqlite3_stmt* GetOrder(int OrderNumber);
sqlite3_stmt* GetOrderList();
int GetOrderSize(int OrderNumber);
bool DeleteOrder(int OrderNumber);

sqlite3_stmt* GetOrderItemList(int OrderNumber);
sqlite3_stmt* GetOrderItemPreviewList(int OrderNumber); 
int GetOrderItemCount(int OrderNumber);
sqlite3_stmt* GetOrderItem(int OrderNumber, int ItemID);
bool UpdateOrderItem(int OrderNumber, int ItemID, int Quantity);
bool DeleteOrderItem(int OrderNumber, int ItemID);

sqlite3_stmt* GetItem(int ItemID);
int GetItemCount();
sqlite3_stmt* GetItemList(); 
