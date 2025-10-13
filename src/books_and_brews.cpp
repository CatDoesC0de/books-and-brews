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
#include <stdio.h>
#include <vector>
#include <string>

void ShowAddOrderMenu()
{
    while (true)
    {
        ClearScreen();
        PrintLogs();

        int MenuItemCount = GetItemCount();
        if (MenuItemCount == 0)
        {
            BB_LOG_ERROR(
                "Failed to create order. No menu items in the system.");
            return;
        }

        int RowsPerPage;
        do
        {
            printf("Select items to add to this order.\n");
            printf("Fetched %i menu items. How many do you want per-page? (-1 "
                   "to cancel)\n",
                   MenuItemCount);
            printf(">> ");
        } while (!ReadInt(RowsPerPage));

        if (RowsPerPage == -1)
        {
            return;
        }

        struct OrderInput
        {
            std::string ItemName;
            int ItemID;
            int Quantity;
        } Order;

        std::vector<OrderInput> ItemsForOrder;
        sqlite3_stmt* MenuItemList = GetItemList();
        
        int Page = 0;
        while (true)
        {
            ClearScreen();
            PrintLogs();

            printf("Items in cart: %lu\n", ItemsForOrder.size());
            printf("Select an item to add. (-2 to cancel, 0 to go to the next "
                   "page, -1 to go back a page)\n");

            for (int i = 0; i < RowsPerPage && Step(MenuItemList); i++)
            {
                statement_reader Reader(MenuItemList);

                int ItemID = Reader.integer();
                Text* ItemName = Reader.text();
                Text* ItemDescription = Reader.text();
                float ItemPrice = Reader.decimal();

                printf("%i. %s [%s] - %.2f\n", ItemID, ItemName,
                       ItemDescription, ItemPrice);
            }

            printf(">> ");

            int Choice;
            if (!ReadInt(Choice))
            {
            }
            else if (Choice == -2)
            {
                return;
            }
            else if (Choice == 0)
            {
                if ((Page * RowsPerPage) + RowsPerPage < MenuItemCount)
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
            else if (Choice < (Page * RowsPerPage) || Choice > (Page * RowsPerPage) + RowsPerPage)
            {
                BB_LOG_ERROR("Invalid choice. Choice not on the current page.");
            }
            else //Actual ItemID path
            {
                Order.ItemID = Choice;

                sqlite3_stmt* Item = GetItem(Order.ItemID);
                Order.ItemName = reinterpret_cast<const char*>(sqlite3_column_text(Item, 1));
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
                }
                while (!ReadBool(AddAnotherItem));

                if (!AddAnotherItem)
                {
                    int64_t OrderNumber;
                    if (!CreateOrder(OrderNumber))
                    {
                        return;
                    }

                    Transaction();
                    for (OrderInput Input : ItemsForOrder)
                    {
                        if (!AddItemToOrder(OrderNumber, Input.ItemID, Input.Quantity))
                        {
                            Rollback();
                            return;
                        }
                    }
                    Commit();

                    BB_LOG_INFO("Order created with ID: %li", OrderNumber);
                    return;
                }
            }

            //Reset to updated page
            sqlite3_reset(MenuItemList);
            for (int i = 0; i < Page * RowsPerPage; i++)
            {
                Step(MenuItemList);
            }
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

int main(int Argc, char** Argv)
{
    const char DatabaseFile[] = "books_and_brews.db";
    if (!DatabaseInit(DatabaseFile))
    {
        return -1;
    }

    LoggerInit(CreateLogger("B&B Logs", 5));

    bool ShouldExit = false;
    while (true && !ShouldExit)
    {
        ClearScreen();
        PrintLogs();

        printf("What would you like to do? (-1 to exit)\n");
        printf("1. Add\n");
        printf(">> ");

        int Choice;

        if (!ReadInt(Choice))
        {
            continue;
        }

        switch (Choice)
        {
            case 1: ShowAddMenu(ShouldExit); break;
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
