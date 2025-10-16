/*
 * -------------------------------
 * Copyright (C) 2025 Connor Taylor.
 * Released under the MIT License.
 * -------------------------------
 *
 * Program name: books_and_brews.cpp
 * Author: Connor Taylor
 * Last Update: 10/16/2025
 * Purpose: Create a software interface for managing the Books and Brews
 * inventory.
 */

#include "database.hpp"
#include "input.hpp"
#include "logger.hpp"
#include <assert.h>
#include <functional>
#include <regex>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

std::regex LettersOnlyRegex("^([A-Za-z ]+|-1)$");

static int
GetPagingSelection(sqlite3_stmt* QueryList, int QueryListCount,
                   const char* RowName,
                   const std::function<void(row_reader)>& RowPrintFunction,
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
            RowPrintFunction(row_reader(QueryList));
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

void ShowAddItemMenu()
{
    std::string ItemName;
    while (true)
    {
        ClearScreen();
        PrintLogs();

        printf("What is the name of the new item? (-1 to cancel)\n");
        printf(">> ");

        if (ReadString(ItemName, LettersOnlyRegex))
        {
            break;
        }
        BB_LOG_ERROR(
            "Invalid item name. Only letters are allowed in the name.");
    }

    if (ItemName == "-1")
    {
        return;
    }

    auto SupplyPrintFunction = [](row_reader Reader) {
        int SupplyID = Reader.integer();
        const char* SupplyName = Reader.text();

        printf("%i. %s\n", SupplyID, SupplyName);
    };

    auto SupplySelectionText = []() {
        printf("Select a supply to add to this item.\n");
    };

    std::vector<ingredient> Ingredients;
    auto SupplyValidation = [&](int SupplyID) {
        for (const ingredient Ingredient : Ingredients)
        {
            if (Ingredient.SupplyID == SupplyID)
            {
                BB_LOG_ERROR("Ingredient already added to this item.");
                return false;
            }
        }

        sqlite3_stmt* SupplyItem = GetSupplyItem(SupplyID);
        sqlite3_finalize(SupplyItem);
        return SupplyItem != nullptr;
    };

    sqlite3_stmt* SupplyList = GetSupplyList();
    int SupplyCount = GetSupplyCount();
    while (true)
    {
        sqlite3_reset(SupplyList);

        int SupplyID = GetPagingSelection(
            SupplyList, SupplyCount, "supply", SupplyPrintFunction,
            SupplySelectionText, SupplyValidation);

        if (SupplyID == -1)
        {
            sqlite3_finalize(SupplyList);
            return;
        }

        sqlite3_stmt* SupplyItem = GetSupplyItem(SupplyID);
        row_reader Reader(SupplyItem);
        Reader.integer(); // Skip ID
        const char* SupplyName = Reader.text();
        Reader.integer(); // Skip Quantity
        const char* SupplyUnitName = Reader.text();

        double IngredientQuantity;
        while (true)
        {
            ClearScreen();
            PrintLogs();

            printf("How much of '%s' does this item use? (Units: %s) (eg. "
                   "'1.5')\n",
                   SupplyName, SupplyUnitName);
            printf(">> ");

            if (ReadPositiveDouble(IngredientQuantity))
            {
                break;
            }

            BB_LOG_ERROR("Quantity must be a positive value");
        }
        sqlite3_finalize(SupplyItem);

        Ingredients.push_back(
            {.SupplyID = SupplyID, .Quantity = IngredientQuantity});

        bool AddAnotherIngredient;
        while (true)
        {
            ClearScreen();
            PrintLogs();

            printf("Add another ingredient? [Y/N]\n");
            printf(">> ");

            if (ReadBool(AddAnotherIngredient))
            {
                break;
            }
        }

        if (!AddAnotherIngredient)
        {
            break;
        }
    }

    std::string ItemDescription;
    while (true)
    {
        ClearScreen();
        PrintLogs();

        printf("What is the description of the '%s'? (-1 to cancel)\n",
               ItemName.c_str());
        printf(">> ");

        if (ReadString(ItemDescription, std::regex("^([A-Za-z .,]+|-1)$")))
        {
            break;
        }
        BB_LOG_ERROR("Invalid item description. Only letters and punctuation "
                     "are allowed in "
                     "the description.");
    }

    if (ItemDescription == "-1")
    {
        return;
    }

    double ItemPrice;
    while (true)
    {
        ClearScreen();
        PrintLogs();

        printf("What is the price of '%s'? (-1 to cancel)\n", ItemName.c_str());
        printf(">> ");

        if (ReadPositiveDouble(ItemPrice))
        {
            break;
        }

        BB_LOG_ERROR("Invalid item price. Expected postive decimal > 0.");
    }

    if (CreateItem(ItemName.c_str(), ItemDescription.c_str(), ItemPrice,
                   Ingredients))
    {
        int64_t ItemID = LastInsertRowID();
        BB_LOG_INFO("Item '%s' created with ID: %i", ItemName.c_str(), ItemID);
    }

    sqlite3_finalize(SupplyList);
}

void ShowAddSupplyMenu()
{
    std::string SupplyName;
    while (true)
    {
        ClearScreen();
        PrintLogs();

        printf("What is the name of the new supply? (-1 to cancel)\n");
        printf(">> ");

        if (ReadString(SupplyName, LettersOnlyRegex))
        {
            break;
        }

        BB_LOG_ERROR(
            "Invalid supply name. Only letters are allowed in the name.");
    }

    if (SupplyName == "-1")
    {
        return;
    }

    std::string UnitName;
    while (true)
    {
        ClearScreen();
        PrintLogs();

        printf("What are the new supply units? (eg. lbs, bags, oz) (-1 to "
               "cancel)\n");
        printf(">> ");

        if (ReadString(UnitName, LettersOnlyRegex))
        {
            break;
        }
        BB_LOG_ERROR(
            "Invalid unit name. Only letters are allowed in the name.");
    }

    if (UnitName == "-1")
    {
        return;
    }

    int Quantity;
    while (true)
    {
        ClearScreen();
        PrintLogs();

        printf("How many units of this supply are currently in stock? (-1 to "
               "cancel)\n");
        printf(">> ");

        if (!ReadInt(Quantity))
        {
            BB_LOG_ERROR("Current stock quantity must be an integer.");
        }
        else if (Quantity >= -1)
        {
            break;
        }
    }

    if (Quantity == -1)
    {
        return;
    }

    if (CreateSupply(SupplyName.c_str(), UnitName.c_str(), Quantity))
    {
        int64_t SupplyID = LastInsertRowID();
        BB_LOG_INFO("Created supply '%s' with ID: %i", SupplyName.c_str(),
                    SupplyID);
    }
}

void ShowAddOrderMenu()
{
    int MenuItemCount = GetItemCount();
    if (MenuItemCount == 0)
    {
        BB_LOG_ERROR("Failed to create order. No menu items in the system.");
        return;
    }

    std::vector<order_input> ItemsForOrder;
    auto ItemPrintCallback = [](row_reader Reader) {
        int ItemID = Reader.integer();
        const char* ItemName = Reader.text();
        const char* ItemDescription = Reader.text();
        float ItemPrice = Reader.decimal();

        printf("%i. %s [%s] - %.2f\n", ItemID, ItemName, ItemDescription,
               ItemPrice);
    };

    auto ItemSelectionText = [&]() {
        printf("Select an item to add [%lu items in order] - ",
               ItemsForOrder.size());
    };

    auto ItemValidation = [&](int ItemID) {
        for (const auto& OrderInput : ItemsForOrder)
        {
            if (OrderInput.ItemID == ItemID)
            {
                BB_LOG_ERROR("Item has already been added to the order.");
                return false;
            }
        }

        sqlite3_stmt* Item = GetItem(ItemID);
        sqlite3_finalize(Item);
        return Item != nullptr;
    };

    sqlite3_stmt* MenuItemList = GetItemList();
    while (true)
    {
        ClearScreen();
        PrintLogs();

        sqlite3_reset(MenuItemList);
        int ItemChoice = GetPagingSelection(MenuItemList, MenuItemCount,
                                            "items", ItemPrintCallback,
                                            ItemSelectionText, ItemValidation);

        if (ItemChoice == -1)
        {
            break;
        }

        sqlite3_stmt* Item = GetItem(ItemChoice);
        const char* ItemName = row_reader(Item, 1).text();

        int Quantity;
        while (true)
        {
            ClearScreen();
            PrintLogs();

            printf("Quantity for %s:\n", ItemName);
            printf(">> ");

            if (ReadInt(Quantity))
            {
                if (Quantity > 0)
                {
                    break;
                }
                BB_LOG_ERROR("Quantity must be a positive integer > 0.");
            }
        }
        sqlite3_finalize(Item);

        ItemsForOrder.push_back({.ItemID = ItemChoice, .Quantity = Quantity});
        bool AddAnotherItem;
        while (true)
        {
            ClearScreen();
            PrintLogs();

            printf("Add another item? [Y/N]\n");
            printf(">> ");

            if (ReadBool(AddAnotherItem))
            {
                break;
            }
        }

        if (!AddAnotherItem && CreateOrder(ItemsForOrder))
        {
            int64_t OrderNumber = LastInsertRowID();
            BB_LOG_INFO("Order created with ID: %li", OrderNumber);

            break;
        }

        sqlite3_finalize(MenuItemList);
    }

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
        printf("2. Supply\n");
        printf("3. Item\n");
        printf(">> ");

        int Choice;
        if (!ReadInt(Choice))
        {
            continue;
        }

        switch (Choice)
        {
            case 1: ShowAddOrderMenu(); break;
            case 2: ShowAddSupplyMenu(); break;
            case 3: ShowAddItemMenu();
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

    auto OrderPrintCallback = [](row_reader Reader) {
        int OrderNumber = Reader.integer();
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

    auto OrderItemCallback = [](row_reader Reader) {
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

int GetItemSelection()
{
    ClearScreen();
    PrintLogs();

    auto ItemPrintCallback = [](row_reader Reader) {
        int ItemID = Reader.integer();
        const char* ItemName = Reader.text();
        Reader.text();
        double ItemPrice = Reader.decimal();

        printf("%i. %s - $%.2lf\n", ItemID, ItemName, ItemPrice);
    };

    auto ItemSelectionText = []() { printf("Select an item to update. "); };

    auto ItemValidation = [](int ItemID) {
        sqlite3_stmt* Item = GetItem(ItemID);
        sqlite3_finalize(Item);
        return Item != nullptr;
    };

    sqlite3_stmt* ItemList = GetItemList();
    int ItemID =
        GetPagingSelection(ItemList, GetItemCount(), "item", ItemPrintCallback,
                           ItemSelectionText, ItemValidation);

    sqlite3_finalize(ItemList);
    return ItemID;
}

int GetIngredientSelection(int ItemID)
{
    ClearScreen();
    PrintLogs();

    auto IngredientPrintCallback = [](row_reader Reader) {
        Reader.integer();
        int SupplyID = Reader.integer();
        double Quantity = Reader.decimal();

        sqlite3_stmt* Supply = GetSupplyItem(SupplyID);

        Reader = row_reader(Supply, 1);
        const char* SupplyName = Reader.text();
        Reader.decimal();
        const char* UnitName = Reader.text();

        printf("%i. %s - %.2lf %s\n", SupplyID, SupplyName, Quantity, UnitName);
    };

    auto IngredientSelectionText = []() {
        printf("Select an ingredient to update. ");
    };

    auto IngredientValidation = [](int SupplyID) {
        sqlite3_stmt* Supply = GetSupplyItem(SupplyID);
        sqlite3_finalize(Supply);
        return Supply != nullptr;
    };

    sqlite3_stmt* IngredientList = GetIngredientList(ItemID);
    int SupplyID = GetPagingSelection(
        IngredientList, GetIngredientCount(ItemID), "item",
        IngredientPrintCallback, IngredientSelectionText, IngredientValidation);

    sqlite3_finalize(IngredientList);
    return SupplyID;
}

void ShowUpdateItemMenu()
{
    int ItemCount = GetItemCount();
    if (ItemCount == 0)
    {
        BB_LOG_ERROR("There are 0 items in the system currently.");
        return;
    }

    int ItemID = GetItemSelection();
    if (ItemID == -1)
    {
        return;
    }

    int SupplyID = GetIngredientSelection(ItemID);
    if (SupplyID == -1)
    {
        return;
    }

    sqlite3_stmt* Supply = GetSupplyItem(ItemID);
    const char* SupplyName = row_reader(Supply, 1).text();

    while (true)
    {
        ClearScreen();
        PrintLogs();

        printf("Remove this supply from this item? Or edit this supply's "
               "quantity? (-1 to cancel)\n");
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
                if (GetIngredientCount(ItemID) - 1 == 0)
                {
                    if (DeleteItem(ItemID))
                    {
                        BB_LOG_INFO("Deleted item #%i", ItemID);
                    }
                }
                else if (DeleteIngredient(ItemID, SupplyID))
                {
                    BB_LOG_INFO("'%s' removed from item #%i", SupplyName,
                                ItemID);
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

                    if (ReadPositiveInt(Quantity))
                    {
                        break;
                        BB_LOG_ERROR("Quantity must be a positive integer.");
                    }
                    BB_LOG_ERROR("Quantity must be a positive integer.");
                }

                if (UpdateIngredient(ItemID, SupplyID, Quantity))
                {
                    BB_LOG_INFO("Ingredient %s updated with quantity %i",
                                SupplyName, Quantity);
                }
                return;
            default:
                BB_LOG_ERROR("Invalid choice. Choice not available.");
                continue;
        }
    }

    sqlite3_finalize(Supply);
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

                    if (ReadPositiveInt(Quantity))
                    {
                        break;
                        BB_LOG_ERROR("Quantity must be a positive integer.");
                    }
                    BB_LOG_ERROR("Quantity must be a positive integer.");
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
        printf("2. Item\n");
        printf(">> ");

        int Choice;
        if (!ReadInt(Choice))
        {
            continue;
        }

        switch (Choice)
        {
            case 1: ShowUpdateOrderMenu(); break;
            case 2: ShowUpdateItemMenu(); break;
            case 0: return;
            case -1: ShouldExit = true; return;
            default:
                BB_LOG_ERROR("Invalid choice. Choice not available.");
                continue;
        }
    }
}

void ShowDeleteMenu(bool& ShouldExit)
{
    while (true)
    {
        ClearScreen();
        PrintLogs();

        printf("What would you like to delete? (-1 to exit, 0 to go back)\n");
        printf("1. Order\n");
        printf("2. Item\n");
        printf(">> ");

        int Choice;
        if (!ReadInt(Choice))
        {
            continue;
        }

        switch (Choice)
        {
            case 1:
            {
                int OrderNumber = GetOrderSelection();
                if (DeleteOrder(OrderNumber))
                {
                    BB_LOG_INFO("Deleted order #%i", OrderNumber);
                }
                return;
            }
            case 2:
            {
                int ItemID = GetItemSelection();
                if (DeleteItem(ItemID))
                {
                    BB_LOG_INFO("Deleted item #%i", ItemID);
                }
                return;
            }
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
        printf("3. Delete\n");
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
            case 3: ShowDeleteMenu(ShouldExit); break;
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
