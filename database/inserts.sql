INSERT INTO SupplyItem(SupplyName, StockQuantity, UnitName) VALUES
    ('Coffee Bean', 5.0, 'lb'),
    ('Almond Milk', 3.0, 'gallon'),
    ('Sugar', 2, 'lb'),
    ('Honey', 32, 'ounce'),
    ('Cinnamon', 1, 'lb');

INSERT INTO Item(ItemName, ItemDescription, ItemPrice) VALUES
    ('Black Coffee', 'Classic brewed coffee beans.', 2.50),
    ('Honey Coffee', 'Brewed coffee with pure honey sweetener.', 3.50),
    ('Iced Latte', 'Coffee with almond milk over ice.', 4.50),
    ('Iced Honey Cinnamon Coffee', 'Iced coffee served with honey and a cinnamon stick.', 5.50),
    ('Cafe Miel', 'Coffee served with steamed almond milk, honey, and cinnamon', 6.50);

INSERT INTO Ingredient(ItemID, SupplyID, Quantity) VALUES

    --Black Coffee
    (1, 1, 0.1),

    --Honey Coffee
    (2, 1, 0.1),
    (2, 3, 0.05),
    (2, 4, 1),

    --Iced Latte
    (3, 1, 0.1),
    (3, 2, 0.05),
    (3, 3, 0.05),

    --Iced Honey Cinnamon Coffee
    (4, 1, 0.1),
    (4, 4, 2),
    (4, 5, 0.025),

    -- Cafe Miel
    (5, 1, 0.1),
    (5, 2, 0.15),
    (5, 4, 2),
    (5, 5, 0.025);


INSERT INTO MenuOrder(OrderDate) VALUES
    (CURRENT_DATE),
    (CURRENT_DATE),
    (CURRENT_DATE),
    (CURRENT_DATE),
    (CURRENT_DATE);

INSERT INTO MenuOrderItem(OrderNumber, ItemID, OrderQuantity) VALUES
    --Order 1
    (1, 1, 1),

    --Order 2
    (1, 4, 1),
    (2, 2, 2),

    --Order 3
    (3, 5, 1),

    --Order 4
    (4, 3, 3);
