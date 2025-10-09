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

#include "database.hpp"
#include "logger.hpp"
#include <stdio.h>

bool Step(sqlite3_stmt* Statement, sqlite3* Database)
{
    if (sqlite3_step(Statement) != SQLITE_ROW)
    {
        BB_LOG_DEBUG(Logger, "Failed to step query. (%s)",
                 sqlite3_errmsg(Database));
        return false;
    }

    return true;
}

sqlite3_stmt* Prepare(const char* Query, sqlite3* Database)
{
    sqlite3_stmt* Statement = nullptr;
    if (sqlite3_prepare_v2(Database, Query, -1, &Statement, nullptr) !=
        SQLITE_OK)
    {
        BB_LOG_DEBUG(Logger, "Failed to prepare query. (%s)",
                 sqlite3_errmsg(Database));
    }

    return Statement;
    ;
}

static int GetCount(const char* Table, sqlite3* Database)
{
    int Result = 0;

    char Query[256];
    snprintf(Query, sizeof(Query), "SELECT COUNT(*) FROM %s;", Table);

    sqlite3_stmt* Statement = Prepare(Query, Database);
    if (Statement != nullptr && Step(Statement, Database))
    {
        Result = sqlite3_column_int(Statement, 0);
    }

    sqlite3_finalize(Statement);
    return Result;
}

static sqlite3_stmt* GetList(const char* Table, sqlite3* Database)
{
    char Query[256];
    snprintf(Query, sizeof(Query), "SELECT * FROM %s", Table);

    sqlite3_stmt* Statement = Prepare(Query, Database);
    if (Statement != nullptr)
    {
        sqlite3_bind_text(Statement, 1, Table, -1, nullptr);
    }

    return Statement;
}

int GetOutgoingOrderCount(sqlite3* Database)
{
    int Result = GetCount("MenuOrder", Database);
    return Result;
}

int GetItemCount(sqlite3* Database)
{
    int Result = GetCount("Item", Database);
    return Result;
}

sqlite3_stmt* GetItemList(sqlite3* Database)
{
    sqlite3_stmt* Result = GetList("Item", Database);
    return Result;
}
