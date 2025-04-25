#ifndef ORDERITEMMODEL_H
#define ORDERITEMMODEL_H

#include <string>
#include <iostream>
#include "db/DatabaseConnection.h"
#include "entities/OrderItem.h"
#include <optional>
#include <vector>
#include <memory>
#include <cstring>
#include <set>

class OrderItemModel
{
private:
    int id;
    int order_id;
    int product_id;
    int quantity;
    double price;

public:
    OrderItemModel() {};
    std::optional<int> createOrderItem(const std::vector<OrderItem> &products, int order_id);
    std::optional<int> updateOrderItems(const std::vector<OrderItem> &products, int order_id);
    std::optional<int> syncOrderItems(const std::vector<OrderItem> &newItems, int order_id);
};

#endif