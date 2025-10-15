/*
 * -------------------------------
 * Copyright (C) 2025 Connor Taylor.
 * Released under the MIT License.
 * -------------------------------
 *
 * Program name: books_and_brews.cpp
 * Author: Connor Taylor
 * Purpose: Define utility functions for interacting with the database
 */

#include "database.hpp"
#include "logger.hpp"
#include <assert.h>
#include <stdio.h>

sqlite3* Database;

bool DatabaseInit(const char* FileName)
{
    if (sqlite3_open_v2(FileName, &Database, SQLITE_OPEN_READWRITE, nullptr) !=
        SQLITE_OK)
    {
        fprintf(stderr, "Failed to open database with file '%s'. (%s)\n",
                FileName, sqlite3_errmsg(Database));
        return false;
    }

    return true;
}

void DatabaseClose()
{
    sqlite3_close_v2(Database);
}

statement_reader::statement_reader(sqlite3_stmt* Statement)
    : Statement(Statement), ReadIndex(0)
{}

int statement_reader::integer()
{
    int Result = sqlite3_column_int(Statement, ReadIndex++);
    return Result;
}

Text* statement_reader::text()
{
    Text* Result = sqlite3_column_text(Statement, ReadIndex++);
    return Result;
}

double statement_reader::decimal()
{
    double Result = sqlite3_column_double(Statement, ReadIndex++);
    return Result;
}

statement_binder::statement_binder(sqlite3_stmt* Statement)
    : Statement(Statement), BindIndex(1)
{}

void statement_binder::integer(int Value)
{
    sqlite3_bind_int(Statement, BindIndex++, Value);
}

void Transaction()
{
    sqlite3_exec(Database, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
}

void Commit()
{
    sqlite3_exec(Database, "COMMIT;", nullptr, nullptr, nullptr);
}

void Rollback()
{
    sqlite3_exec(Database, "ROLLBACK;", nullptr, nullptr, nullptr);
}

int64_t LastInsertRowID()
{
    return sqlite3_last_insert_rowid(Database);
}

static bool _Execute(sqlite3_stmt* Statement)
{
    if (sqlite3_step(Statement) != SQLITE_DONE)
    {
        BB_LOG_ERROR("Failed to step query. (%s)", sqlite3_errmsg(Database));
        return false;
    }

    return true;
}

bool StepRow(sqlite3_stmt* Statement)
{
    int StepResult = sqlite3_step(Statement);

    if (StepResult == SQLITE_DONE)
    {
        return false; //No more rows, return false
    }

    if (StepResult != SQLITE_ROW) //Not returning a row, log the error
    {
        BB_LOG_ERROR("Failed to step query. (%s)", sqlite3_errmsg(Database));
        return false;
    }

    return true;
}

sqlite3_stmt* Prepare(const char* Query)
{
    sqlite3_stmt* Statement = nullptr;
    if (sqlite3_prepare_v2(Database, Query, -1, &Statement, nullptr) !=
        SQLITE_OK)
    {
        BB_LOG_DEBUG("Failed to prepare query. '%s' (%s)", Query,
                     sqlite3_errmsg(Database));
        Statement = nullptr;
    }
    return Statement;
}

static int _GetCount(const char* Table)
{
    int Result = 0;

    char Query[256];
    snprintf(Query, sizeof(Query), "SELECT COUNT(*) FROM %s;", Table);

    sqlite3_stmt* Statement = Prepare(Query);
    if (Statement != nullptr && StepRow(Statement))
    {
        Result = sqlite3_column_int(Statement, 0);
    }

    sqlite3_finalize(Statement);
    return Result;
}

static sqlite3_stmt* _GetList(const char* Table)
{
    char Query[256];
    snprintf(Query, sizeof(Query), "SELECT * FROM %s", Table);
    sqlite3_stmt* Statement = Prepare(Query);
    return Statement;
}

bool CreateOrder(int64_t& Result)
{
    sqlite3_stmt* Statement =
        Prepare("INSERT INTO MenuOrder (OrderDate) VALUES (current_date)");

    bool Created = false;
    if (Statement != nullptr && _Execute(Statement))
    {
        Result = LastInsertRowID();
        Created = true;
    }
    else
    {
        BB_LOG_ERROR("Failed to create a new order.");
    }

    sqlite3_finalize(Statement);
    return Created;
}

bool AddItemToOrder(int OrderNumber, int ItemID, int ItemQuantity)
{
    sqlite3_stmt* Statement =
        Prepare("INSERT INTO MenuOrderItem (OrderNumber, ItemID, OrderQuantity)"
                "VALUES (?, ?, ?)");

    bool Result = false;
    if (Statement != nullptr)
    {
        statement_binder Binder(Statement);

        Binder.integer(OrderNumber);
        Binder.integer(ItemID);
        Binder.integer(ItemQuantity);

        Result = _Execute(Statement);
    }

    sqlite3_finalize(Statement);
    return Result;
}

int GetOrderCount()
{
    int Result = _GetCount("MenuOrder");
    return Result;
}

sqlite3_stmt* GetOrder(int OrderNumber)
{
    sqlite3_stmt* Statement =
        Prepare("SELECT * FROM MenuOrder WHERE OrderNumber = ?");

    if (Statement != nullptr)
    {
        sqlite3_bind_int(Statement, 1, OrderNumber);

        if (!StepRow(Statement))
        {
            BB_LOG_ERROR("Failed to retrieve MenuOrder with OrderNumber = %i",
                         OrderNumber);
            sqlite3_finalize(Statement);
            Statement = nullptr;
        }
    }

    return Statement;
}

sqlite3_stmt* GetOrderList()
{
    return _GetList("MenuOrder");
}

sqlite3_stmt* GetOrderItemPreviewList(int OrderNumber)
{
    sqlite3_stmt* Statement = Prepare(R"(
        SELECT
        MenuOrderItem.OrderQuantity,
        Item.ItemID,
        Item.ItemName
        FROM MenuOrderItem
        JOIN Item ON MenuOrderItem.ItemID = Item.ItemID
        WHERE MenuOrderItem.OrderNumber = ?
    )");

    if (Statement != nullptr)
    {
        sqlite3_bind_int(Statement, 1, OrderNumber);
    }

    return Statement;
}

int GetOrderItemCount(int OrderNumber)
{
    sqlite3_stmt* Statement =
        Prepare("SELECT COUNT(*) FROM MenuOrderItem WHERE OrderNumber = ?");

    int Result = 0;
    if (Statement != nullptr)
    {
        sqlite3_bind_int(Statement, 1, OrderNumber);

        if (StepRow(Statement))
        {
            Result = sqlite3_column_int(Statement, 0);
        }
    }

    sqlite3_finalize(Statement);
    return Result;
}

sqlite3_stmt* GetOrderItem(int OrderNumber, int ItemID)
{
    sqlite3_stmt* Statement = Prepare(R"(
        SELECT * 
        FROM MenuOrderItem
        WHERE OrderNumber = ? AND ItemID = ?
    )");

    if (Statement != nullptr)
    {
        sqlite3_bind_int(Statement, 1, OrderNumber);
        sqlite3_bind_int(Statement, 2, ItemID);

        if (!StepRow(Statement))
        {
            BB_LOG_ERROR("Failed to retrieve MenuOrderItem with OrderNumber = "
                         "%i, ItemID = %i",
                         OrderNumber, ItemID);

            sqlite3_finalize(Statement);
            Statement = nullptr;
        }
    }

    return Statement;
}

sqlite3_stmt* GetItem(int ItemID)
{
    sqlite3_stmt* Statement = Prepare("SELECT * FROM Item WHERE ItemID = ?");

    if (Statement != nullptr)
    {
        sqlite3_bind_int(Statement, 1, ItemID);

        if (!StepRow(Statement))
        {
            BB_LOG_ERROR("Failed to retrieve Item with ItemID = %i", ItemID);
            sqlite3_finalize(Statement);
            Statement = nullptr;
        }
    }

    return Statement;
}

int GetItemCount()
{
    int Result = _GetCount("Item");
    return Result;
}

sqlite3_stmt* GetItemList()
{
    sqlite3_stmt* Result = _GetList("Item");
    return Result;
}
