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
#include <stdio.h>

sqlite3* Database;

bool DatabaseInit(const char* FileName)
{
    if (sqlite3_open_v2(FileName, &Database, SQLITE_OPEN_READWRITE, nullptr) !=
        SQLITE_OK)
    {
        fprintf(stderr, "Failed to open database with file '%s'. (%s)",
                FileName, sqlite3_errmsg(Database));
        return false;
    }

    return true;
}

void DatabaseClose()
{
    sqlite3_close_v2(Database);
}

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

bool Step(sqlite3_stmt* Statement)
{
    if (sqlite3_step(Statement) != SQLITE_ROW)
    {
        BB_LOG_DEBUG("Failed to step query. (%s)", sqlite3_errmsg(Database));
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
        BB_LOG_DEBUG("Failed to prepare query. (%s)", sqlite3_errmsg(Database));
    }

    return Statement;
    ;
}

static int GetCount(const char* Table)
{
    int Result = 0;

    char Query[256];
    snprintf(Query, sizeof(Query), "SELECT COUNT(*) FROM %s;", Table);

    sqlite3_stmt* Statement = Prepare(Query);
    if (Statement != nullptr && Step(Statement))
    {
        Result = sqlite3_column_int(Statement, 0);
    }

    sqlite3_finalize(Statement);
    return Result;
}

static sqlite3_stmt* GetList(const char* Table)
{
    char Query[256];
    snprintf(Query, sizeof(Query), "SELECT * FROM %s", Table);

    sqlite3_stmt* Statement = Prepare(Query);
    if (Statement != nullptr)
    {
        sqlite3_bind_text(Statement, 1, Table, -1, nullptr);
    }

    return Statement;
}

int GetOutgoingOrderCount()
{
    int Result = GetCount("MenuOrder");
    return Result;
}

int GetItemCount()
{
    int Result = GetCount("Item");
    return Result;
}

sqlite3_stmt* GetItemList()
{
    sqlite3_stmt* Result = GetList("Item");
    return Result;
}
