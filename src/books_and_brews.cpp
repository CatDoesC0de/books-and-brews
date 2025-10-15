/*
 * -------------------------------
 * Copyright (C) 2025 Connor Taylor.
 * Released under the MIT License.
 * -------------------------------
 *
 * Program name: books_and_brews.cpp
 * Author: Connor Taylor
 * Purpose: Create a software interface for managing the Books and Brews
 * inventory.
 */

#include "database.hpp"
#include "input.hpp"
#include "logger.hpp"
#include <assert.h>
#include <functional>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

static int
GetPagingSelection(sqlite3_stmt* QueryList, int QueryListCount,
                   const char* RowName,
                   const std::function<void(sqlite3_stmt*)>& RowPrintFunction,
                   const std::function<void()>& SelectionText,
                   const std::function<bool(int Selection)>& Validate)
{
    int RowsPerPage;
    do
    {
        ClearScreen();
        PrintLogs();

        printf("Fetched %i %s. How many do you want per-page? (-1 "
               "to cancel)\n",
               QueryListCount, RowName);
        printf(">> ");
    } while (!ReadInt(RowsPerPage));

    if (RowsPerPage == -1)
    {
        return -1;
    }

    int Page = 0;
    while (true)
    {
        ClearScreen();
        PrintLogs();

        SelectionText();
        printf("(-2 to cancel, 0 to go to the next "
               "page, -1 to go back a page)\n");

        for (int i = 0; i < RowsPerPage && StepRow(QueryList); i++)
        {
            RowPrintFunction(QueryList);
        }

        printf(">> ");

        int Choice;
        if (!ReadInt(Choice))
        {}
        else if (Choice == -2)
        {
            return -1;
        }
        else if (Choice == 0)
        {
            if ((Page * RowsPerPage) + RowsPerPage < QueryListCount)
            {
                Page++;
            }
        }
        else if (Choice == -1)
        {
            if (Page > 0)
            {
                Page--;
            }
        }
        else
        {
            if (Validate(Choice))
            {
                return Choice;
            }
        }

        // Reset to updated page
        sqlite3_reset(QueryList);
        for (int i = 0; i < Page * RowsPerPage; i++)
        {
            StepRow(QueryList);
        }
    }

    assert(true && "Never should reach this point.");
}

void ShowAddOrderMenu()
{
    ClearScreen();
    PrintLogs();

    int MenuItemCount = GetItemCount();
    if (MenuItemCount == 0)
    {
        BB_LOG_ERROR("Failed to create order. No menu items in the system.");
        return;
    }

    struct order_input
    {
        std::string ItemName;
        int ItemID;
        int Quantity;
    };

    std::vector<order_input> ItemsForOrder;
    auto ItemPrintCallback = [](sqlite3_stmt* ItemRow) {
        statement_reader Reader(ItemRow);

        int ItemID = Reader.integer();
        Text* ItemName = Reader.text();
        Text* ItemDescription = Reader.text();
        float ItemPrice = Reader.decimal();

        printf("%i. %s [%s] - %.2f\n", ItemID, ItemName, ItemDescription,
               ItemPrice);
    };

    auto ItemSelectionText = [&]() {
        printf("Select an item to add [%lu items in order] - ",
               ItemsForOrder.size());
    };

    auto ItemValidation = [](int ItemID) {
        sqlite3_stmt* Item = GetItem(ItemID);
        sqlite3_finalize(Item);
        return Item != nullptr;
    };

    sqlite3_stmt* MenuItemList = GetItemList();

    while (true)
    {
        int ItemChoice = GetPagingSelection(MenuItemList, MenuItemCount,
                                            "items", ItemPrintCallback,
                                            ItemSelectionText, ItemValidation);

        if (ItemChoice == -1)
        {
            break;
        }

        sqlite3_stmt* Item = GetItem(ItemChoice);
        order_input Order;
        Order.ItemID = ItemChoice;
        Order.ItemName =
            reinterpret_cast<const char*>(sqlite3_column_text(Item, 1));
        sqlite3_finalize(Item);

        while (true)
        {
            ClearScreen();
            PrintLogs();

            printf("Quantity for %s:\n", Order.ItemName.c_str());
            printf(">> ");

            if (ReadInt(Order.Quantity))
            {
                if (Order.Quantity <= 0)
                {
                    BB_LOG_ERROR("Quantity must be a positive integer.");
                }
                else
                {
                    break;
                }
            }
        }

        ItemsForOrder.push_back(Order);

        bool AddAnotherItem;
        do
        {
            ClearScreen();
            PrintLogs();

            printf("Add another item? [Y/N]\n");
            printf(">> ");
        } while (!ReadBool(AddAnotherItem));

        if (!AddAnotherItem)
        {
            int64_t OrderNumber;
            if (!CreateOrder(OrderNumber))
            {
                goto cleanup;
            }

            Transaction();
            for (order_input Input : ItemsForOrder)
            {
                if (!AddItemToOrder(OrderNumber, Input.ItemID, Input.Quantity))
                {
                    Rollback();
                    goto cleanup;
                }
            }
            Commit();

            BB_LOG_INFO("Order created with ID: %li", OrderNumber);
        }
    }

cleanup:
    sqlite3_finalize(MenuItemList);
}

void ShowAddMenu(bool& ShouldExit)
{
    while (true)
    {
        ClearScreen();
        PrintLogs();

        printf("What would you like to add? (-1 to exit, 0 to go back)\n");
        printf("1. Order\n");
        printf("2. Menu Item\n");
        printf(">> ");

        int Choice;
        if (!ReadInt(Choice))
        {
            continue;
        }

        switch (Choice)
        {
            case 1: ShowAddOrderMenu(); break;
            case 2:
                // Show add menu item menu
                break;
            case 0: return;
            case -1: ShouldExit = true; return;
            default:
                BB_LOG_ERROR("Invalid choice. Choice not available.");
                continue;
        }
    }
}

int GetOrderSelection()
{
    ClearScreen();
    PrintLogs();

    auto OrderPrintCallback = [](sqlite3_stmt* OrderRow) {
        int OrderNumber = sqlite3_column_int(OrderRow, 0);
        sqlite3_stmt* PreviewList = GetOrderItemPreviewList(OrderNumber);

        printf("#%i | ", OrderNumber);
        while (StepRow(PreviewList))
        {
            int OrderQuantity = sqlite3_column_int(PreviewList, 0);
            const char* ItemName =
                (const char*)sqlite3_column_text(PreviewList, 2);
            printf("x%i - %s ", OrderQuantity, ItemName);
        }
        printf("\n");

        sqlite3_finalize(PreviewList);
    };

    auto OrderSelectionText = []() { printf("Select an order to update.\n"); };

    auto OrderValidation = [](int OrderNumber) {
        sqlite3_stmt* Order = GetOrder(OrderNumber);
        sqlite3_finalize(Order);
        return Order != nullptr;
    };

    sqlite3_stmt* OrderList = GetOrderList();
    int OrderNumber = GetPagingSelection(OrderList, GetOrderCount(), "order",
                                         OrderPrintCallback, OrderSelectionText,
                                         OrderValidation);

    sqlite3_finalize(OrderList);
    return OrderNumber;
}

int GetOrderItemSelection(int OrderNumber)
{
    int OrderItemID;
    ClearScreen();
    PrintLogs();

    auto OrderItemCallback = [](sqlite3_stmt* OrderPreviewRow) {
        int OrderQuantity = sqlite3_column_int(OrderPreviewRow, 0);
        int ItemID = sqlite3_column_int(OrderPreviewRow, 1);
        const char* ItemName =
            (const char*)sqlite3_column_text(OrderPreviewRow, 2);
        printf("%i. %s (x%i)\n", ItemID, ItemName, OrderQuantity);
    };

    auto SelectionText = []() { printf("Select an item to update.\n"); };

    auto OrderItemValidation = [&](int ItemID) {
        sqlite3_stmt* OrderItem = GetOrderItem(OrderNumber, ItemID);
        sqlite3_finalize(OrderItem);
        return OrderItem != nullptr;
    };

    sqlite3_stmt* PreviewList = GetOrderItemPreviewList(OrderNumber);
    OrderItemID = GetPagingSelection(
        PreviewList, GetOrderItemCount(OrderNumber), "order item",
        OrderItemCallback, SelectionText, OrderItemValidation);

    return OrderItemID;
}

void ShowUpdateOrderMenu()
{
    int OrderCount = GetOrderCount();
    if (OrderCount == 0)
    {
        BB_LOG_ERROR("There are 0 orders in the system currently.");
        return;
    }

    int OrderNumber = GetOrderSelection();
    if (OrderNumber == -1)
    {
        return;
    }

    int OrderItemID = GetOrderItemSelection(OrderNumber);
    if (OrderItemID == -1)
    {
        return;
    }
    while (true)
    {
        ClearScreen();
        PrintLogs();

        printf(
            "Remove this item? Or edit this item's quantity? (-1 to cancel)\n");
        printf("1. Remove\n");
        printf("2. Edit Quantity\n");

        int Choice;
        if (!ReadInt(Choice))
        {
            continue;
        }

        switch (Choice)
        {
            case -1: return;
            case 1: 
                //TODO: Remove Order Item from DB
                BB_LOG_INFO("Order item removed from order #%i", OrderNumber);
                return; 
            case 2:
                // TODO: Prompt for quantity, update quantity
                BB_LOG_INFO("Order item updated for order #%i", OrderNumber);
                return; 
            default:
                BB_LOG_ERROR("Invalid choice. Choice not available.");
                continue;
        }
    }
}

void ShowUpdateMenu(bool& ShouldExit)
{
    while (true)
    {
        ClearScreen();
        PrintLogs();

        printf("What would you like to update? (-1 to exit, 0 to go back)\n");
        printf("1. Order\n");
        printf("2. Menu Item\n");
        printf(">> ");

        int Choice;
        if (!ReadInt(Choice))
        {
            continue;
        }

        switch (Choice)
        {
            case 1: ShowUpdateOrderMenu(); break;
            case 2:
                // Show update menu item menu
                break;
            case 0: return;
            case -1: ShouldExit = true; return;
            default:
                BB_LOG_ERROR("Invalid choice. Choice not available.");
                continue;
        }
    }
}

int main(int Argc, char** Argv)
{
    const char DatabaseFile[] = "books_and_brews.db";
    if (!DatabaseInit(DatabaseFile))
    {
        return -1;
    }

    LoggerInit(CreateLogger("B&B Logs", 5));

    bool ShouldExit = false;
    while (!ShouldExit)
    {
        ClearScreen();
        PrintLogs();

        printf("What would you like to do? (-1 to exit)\n");
        printf("1. Add\n");
        printf("2. Update\n");
        printf(">> ");

        int Choice;

        if (!ReadInt(Choice))
        {
            continue;
        }

        switch (Choice)
        {
            case 1: ShowAddMenu(ShouldExit); break;
            case 2: ShowUpdateMenu(ShouldExit); break;
            case -1: goto cleanup; break;
            default:
                BB_LOG_ERROR("Invalid choice. Choice not available.");
                continue;
        }
    }

cleanup:
    printf("Exiting...\n");
    FreeLogger();
    return 0;
}
