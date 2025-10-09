PRAGMA foreign_keys = ON; --Turn on foreign key constraints

CREATE TABLE IF NOT EXISTS SupplyItem (
    SupplyID      INTEGER   NOT NULL PRIMARY KEY,
    SupplyName    TEXT      NOT NULL,
    StockQuantity INTEGER   NOT NULL,
    UnitName      TEXT      NOT NULL,

    UNIQUE(SupplyName)
);

CREATE TABLE IF NOT EXISTS Ingredient (
    ItemID   INTEGER NOT NULL,
    SupplyID INTEGER NOT NULL,
    Quantity INTEGER NOT NULL,

    PRIMARY KEY(ItemID, SupplyID),

    FOREIGN KEY(SupplyID) REFERENCES SupplyItem(SupplyID)
    FOREIGN KEY(ItemID)   REFERENCES Item(ItemID)
);

CREATE TABLE IF NOT EXISTS Item (
    ItemID          INTEGER   NOT NULL PRIMARY KEY,
    ItemName        TEXT      NOT NULL,
    ItemDescription TEXT,
    ItemPrice       FLOAT     NOT NULL,

    UNIQUE(ItemName)
);

CREATE TABLE IF NOT EXISTS MenuOrderItem (
    OrderNumber   INTEGER NOT NULL,
    ItemID        INTEGER NOT NULL,
    OrderQuantity INTEGER NOT NULL,

    PRIMARY KEY(OrderNumber, ItemID),

    FOREIGN KEY(OrderNumber) REFERENCES MenuOrder(OrderNumber),
    FOREIGN KEY(ItemID)      REFERENCES Item(ItemID)
);

CREATE TABLE IF NOT EXISTS MenuOrder (
    OrderNumber INTEGER  NOT NULL PRIMARY KEY,
    OrderDate   TEXT     NOT NULL
);
