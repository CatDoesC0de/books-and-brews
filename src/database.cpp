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
#include <unistd.h>

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

row_reader::row_reader(sqlite3_stmt* Statement) : row_reader(Statement, 0) {}
row_reader::row_reader(sqlite3_stmt* Statement, int ReadIndex)
    : Statement(Statement), ReadIndex(ReadIndex)
{}

int row_reader::integer()
{
    int Result = sqlite3_column_int(Statement, ReadIndex++);
    return Result;
}

const char* row_reader::text()
{
    const char* Result = reinterpret_cast<const char*>(
        sqlite3_column_text(Statement, ReadIndex++));
    return Result;
}

double row_reader::decimal()
{
    double Result = sqlite3_column_double(Statement, ReadIndex++);
    return Result;
}

statement_binder::statement_binder(sqlite3_stmt* Statement)
    : Statement(Statement), BindIndex(1)
{}

statement_binder& statement_binder::integer(int Value)
{
    sqlite3_bind_int(Statement, BindIndex++, Value);
    return *this;
}

statement_binder& statement_binder::decimal(double Value)
{
    sqlite3_bind_double(Statement, BindIndex++, Value);
    return *this;
}

statement_binder& statement_binder::text(const char* Text)
{
    sqlite3_bind_text(Statement, BindIndex++, Text, -1, nullptr);
    return *this;
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
        return false; // No more rows, return false
    }

    if (StepResult != SQLITE_ROW) // Not returning a row, log the error
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

bool CreateOrder(std::vector<order_input>& Items)
{
    sqlite3_stmt* Statement =
        Prepare("INSERT INTO MenuOrder (OrderDate) VALUES (current_date)");

    Transaction();
    bool Result = true;
    if (Statement == nullptr || !(Result = _Execute(Statement)))
    {
        BB_LOG_ERROR("Failed to create a new order.");
    }
    sqlite3_finalize(Statement);

    int64_t OrderNumber = LastInsertRowID();
    if (Result)
    {
        Statement = Prepare(
            "INSERT INTO MenuOrderItem (OrderNumber, ItemID, OrderQuantity)"
            "VALUES (?, ?, ?)");

        if (Statement != nullptr)
        {
            for (order_input Input : Items)
            {

                statement_binder(Statement)
                    .integer(OrderNumber)
                    .integer(Input.ItemID)
                    .integer(Input.Quantity);

                if (!(Result = _Execute(Statement)))
                {
                    break;
                }

                sqlite3_reset(Statement);
                sqlite3_clear_bindings(Statement);
            }
        }
    }

    sqlite3_finalize(Statement);
    Result ? Commit() : Rollback();
    return Result;
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
        Binder.integer(OrderNumber).integer(ItemID).integer(ItemQuantity);
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
        statement_binder(Statement).integer(OrderNumber);

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

bool DeleteOrder(int OrderNumber)
{
    Transaction();
    sqlite3_stmt* OrderItemList = GetOrderItemList(OrderNumber);

    bool Result = true;
    while (StepRow(OrderItemList))
    {
        int ItemID = row_reader(OrderItemList, 1).integer();
        if (!DeleteOrderItem(OrderNumber, ItemID))
        {
            Result = false;
            break;
        }
    }

    sqlite3_stmt* DeleteOrderStatement = nullptr;
    if (Result)
    {
        DeleteOrderStatement =
            Prepare("DELETE FROM MenuOrder WHERE OrderNumber = ?");

        if (DeleteOrderStatement != nullptr)
        {
            statement_binder(DeleteOrderStatement).integer(OrderNumber);
            if (!(Result = _Execute(DeleteOrderStatement)))
            {
                BB_LOG_ERROR("Failed to delete order with OrderNumber = %i",
                             OrderNumber);
            }
        }
    }

    if (Result)
    {
        Commit();
    }
    else
    {
        Rollback();
    }

    sqlite3_finalize(DeleteOrderStatement);
    sqlite3_finalize(OrderItemList);

    return Result;
}

int GetOrderSize(int OrderNumber)
{
    sqlite3_stmt* Statement = Prepare(R"(
        SELECT COUNT(*)
        FROM MenuOrderItem
        WHERE OrderNumber = ?
    )");

    int Result = 0;
    if (Statement != nullptr)
    {
        statement_binder(Statement).integer(OrderNumber);
        if (StepRow(Statement))
        {
            Result = row_reader(Statement).integer();
        }
    }

    return Result;
}

sqlite3_stmt* GetOrderItemList(int OrderNumber)
{
    sqlite3_stmt* Statement =
        Prepare("SELECT * FROM MenuOrderItem WHERE OrderNumber = ?");
    if (Statement != nullptr)
    {
        statement_binder(Statement).integer(OrderNumber);
    }
    return Statement;
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
        statement_binder(Statement).integer(OrderNumber);
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
        statement_binder(Statement).integer(OrderNumber);

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
        statement_binder(Statement).integer(OrderNumber).integer(ItemID);

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

bool UpdateOrderItem(int OrderNumber, int ItemID, int Quantity)
{
    sqlite3_stmt* Statement = Prepare(R"(
        UPDATE MenuOrderItem
        SET OrderQuantity = ?
        WHERE OrderNumber = ? AND ItemID = ?
    )");

    bool Result = false;
    if (Statement != nullptr)
    {
        statement_binder(Statement)
            .integer(Quantity)
            .integer(OrderNumber)
            .integer(ItemID);

        if (!(Result = _Execute(Statement)))
        {
            BB_LOG_ERROR(
                "Failed to update item for OrderNumber = %i, ItemID = %i",
                OrderNumber, ItemID);
        }
    }

    sqlite3_finalize(Statement);
    return Result;
}

bool DeleteOrderItem(int OrderNumber, int ItemID)
{
    sqlite3_stmt* Statement = Prepare(
        "DELETE FROM MenuOrderItem WHERE OrderNumber = ? AND ItemID = ?");

    bool Result = false;
    if (Statement != nullptr)
    {
        statement_binder(Statement).integer(OrderNumber).integer(ItemID);
        if (!(Result = _Execute(Statement)))
        {
            BB_LOG_ERROR(
                "Failed to remove item for OrderNumber = %i, ItemID = %i",
                OrderNumber, ItemID);
        }
    }

    return Result;
}

sqlite3_stmt* GetItem(int ItemID)
{
    sqlite3_stmt* Statement = Prepare("SELECT * FROM Item WHERE ItemID = ?");

    if (Statement != nullptr)
    {
        statement_binder(Statement).integer(ItemID);

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

bool CreateItem(const char* ItemName, const char* ItemDescription,
                float ItemPrice, const std::vector<ingredient>& Ingredients)
{
    bool Result = true;
    sqlite3_stmt* Statement = Prepare(R"(
        INSERT INTO Item (ItemName, ItemDescription, ItemPrice)
        VALUES (?, ?, ?)
    )");

    Transaction();
    int64_t ItemID;
    if (Statement != nullptr)
    {
        statement_binder(Statement)
            .text(ItemName)
            .text(ItemDescription)
            .decimal(ItemPrice);

        if (!(Result = _Execute(Statement)))
        {
            BB_LOG_ERROR("Failed to create item '%s'", ItemName);
        }
    }

    sqlite3_finalize(Statement);
    if (Result)
    {
        Result = true;
        ItemID = LastInsertRowID();

        Statement = Prepare(R"(
            INSERT INTO Ingredient (ItemID, SupplyID, Quantity)
            VALUES (?, ?, ?)
        )");

        if (Statement != nullptr)
        {
            for (const ingredient Ingredient : Ingredients)
            {
                BB_LOG_DEBUG("ItemID = %i SupplyID = %i, Quantity = %i", ItemID,
                             Ingredient.SupplyID, Ingredient.Quantity);
                statement_binder(Statement)
                    .integer(ItemID)
                    .integer(Ingredient.SupplyID)
                    .decimal(Ingredient.Quantity);

                if (!(Result = _Execute(Statement)))
                {
                    break;
                }

                sqlite3_reset(Statement);
                sqlite3_clear_bindings(Statement);
            }
        }
    }

    sqlite3_finalize(Statement);
    Result ? Commit() : Rollback();
    return Result;
}

bool CreateSupply(const char* SupplyName, const char* UnitName, int Quantity)
{
    sqlite3_stmt* Statement = Prepare(R"(
        INSERT INTO SupplyItem(SupplyName, StockQuantity, UnitName)
        VALUES (?, ?, ?)
    )");

    bool Result = false;
    if (Statement != nullptr)
    {
        statement_binder(Statement)
            .text(SupplyName)
            .integer(Quantity)
            .text(UnitName);

        if (!(Result = _Execute(Statement)))
        {
            BB_LOG_ERROR(
                "Failed to create a supply item from\n"
                "\t SupplyName = %s, StockQuantity = %s, UnitName = %s",
                SupplyName, Quantity, UnitName);
        }
    }

    return Result;
}

sqlite3_stmt* GetSupplyItem(int SupplyID)
{
    sqlite3_stmt* Statement =
        Prepare("SELECT * FROM SupplyItem WHERE SupplyID = ?");

    if (Statement != nullptr)
    {
        statement_binder(Statement).integer(SupplyID);

        if (!StepRow(Statement))
        {
            BB_LOG_ERROR("Failed to find SupplyItem with SupplyID = %i",
                         SupplyID);
        }
    }

    return Statement;
}

sqlite3_stmt* GetSupplyList()
{
    return _GetList("SupplyItem");
}

int GetSupplyCount()
{
    return _GetCount("SupplyItem");
}
