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
        row_reader Reader(ItemRow);

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

    auto ItemValidation = [&](int ItemID) {
        bool Result = true;
        sqlite3_stmt* Item = GetItem(ItemID);

        Result = Item != nullptr;
        if (Result)
        {
            for (const auto& OrderInput : ItemsForOrder)
            {
                if (OrderInput.ItemID == ItemID)
                {
                    BB_LOG_ERROR("%s has already been added to the order.",
                                 OrderInput.ItemName.c_str());
                    Result = false;
                    break;
                }
            }
        }

        sqlite3_finalize(Item);
        return Result;
    };

    sqlite3_stmt* MenuItemList;
    while (true)
    {
        MenuItemList = GetItemList();
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
        Order.ItemName = row_reader(Item, 1).text();
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
                return;
            }

            Transaction();
            for (order_input Input : ItemsForOrder)
            {
                if (!AddItemToOrder(OrderNumber, Input.ItemID, Input.Quantity))
                {
                    Rollback();
                    break;
                }
            }

            Commit();
            BB_LOG_INFO("Order created with ID: %li", OrderNumber);
            sqlite3_finalize(MenuItemList);
            break; 
        }

        sqlite3_finalize(MenuItemList);
    }
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
        int OrderNumber = row_reader(OrderRow).integer();
        sqlite3_stmt* PreviewList = GetOrderItemPreviewList(OrderNumber);

        printf("#%i | ", OrderNumber);
        while (StepRow(PreviewList))
        {
            row_reader Reader(PreviewList);
            int OrderQuantity = Reader.integer();
            Reader.integer();
            const char* ItemName = Reader.text();

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
        row_reader Reader(OrderPreviewRow);
        int OrderQuantity = Reader.integer();
        int ItemID = Reader.integer();
        const char* ItemName = Reader.text();

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

    int ItemID = GetOrderItemSelection(OrderNumber);
    if (ItemID == -1)
    {
        return;
    }

    sqlite3_stmt* Item = GetItem(ItemID);
    const char* ItemName = row_reader(Item, 1).text();

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
                if (GetOrderSize(OrderNumber) - 1 == 0)
                {
                    if (DeleteOrder(OrderNumber))
                    {
                        BB_LOG_INFO("Deleted order #%i", OrderNumber);
                    }
                }
                else if (DeleteOrderItem(OrderNumber, ItemID))
                {
                    BB_LOG_INFO("%s removed from order #%i", ItemName,
                                OrderNumber);
                }
                return;
            case 2:
                int Quantity;
                while (true)
                {
                    ClearScreen();
                    PrintLogs();

                    printf("New Quantity? \n");
                    printf(">> ");
                    if (ReadInt(Quantity))
                    {
                        if (Quantity <= 0)
                        {
                            BB_LOG_ERROR(
                                "Quantity must be a positive integer.");
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                if (UpdateOrderItem(OrderNumber, ItemID, Quantity))
                {
                    BB_LOG_INFO("Item %s updated with quantity %i", ItemName,
                                Quantity);
                }
                return;
            default:
                BB_LOG_ERROR("Invalid choice. Choice not available.");
                continue;
        }
    }

    sqlite3_finalize(Item);
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
