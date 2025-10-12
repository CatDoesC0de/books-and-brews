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
        while (true)
        {
            printf("Fetched %i menu items. How many do you want per-page? (-1 "
                   "to cancel)\n",
                   MenuItemCount);
            printf(">> ");

            if (ReadInt(RowsPerPage))
            {
                break;
            }
        }

        if (RowsPerPage == -1)
        {
            return;
        }

        std::vector<int> ItemsForOrder;
        int Page = 0;
        sqlite3_stmt* MenuItemList = GetItemList();
        while (true)
        {
            ClearScreen();
            PrintLogs();

            printf("Select an item to add. (-2 to cancel, 0 to go to the next "
                   "page, -1 to go back a page)\n");

            for (int i = 0; i < RowsPerPage && Step(MenuItemList); i++)
            {
                statement_reader Reader = {};
                Reader.Statement = MenuItemList;

                int ItemID = Reader.integer();
                Text* ItemName = Reader.text();
                Text* ItemDescription = Reader.text();
                float ItemPrice = Reader.decimal();

                printf("%i. %s [%s] - %.2f\n", ItemID, ItemName,
                       ItemDescription, ItemPrice);
            }

            printf(">> ");

            int ItemID;
            if (!ReadInt(ItemID))
            {
            }
            else if (ItemID == -2)
            {
                return;
            }
            else if (ItemID == 0)
            {
                if ((Page * RowsPerPage) + RowsPerPage < MenuItemCount)
                {
                    Page++;
                }
            }
            else if (ItemID == -1)
            {
                if (Page > 0)
                {
                    Page--;
                }
            }
            else if (ItemID < (Page * RowsPerPage) || ItemID > (Page * RowsPerPage) + RowsPerPage - 1)
            {
                BB_LOG_ERROR("Invalid choice. Choice not on the current page.");
            }
            else 
            {
                //Handle quantity then prompt for another item.
            }

            //Reset to updated page
            sqlite3_reset(MenuItemList);
            for (int i = 0; i < Page * RowsPerPage; i++)
            {
                Step(MenuItemList);
            }
        }

        
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
