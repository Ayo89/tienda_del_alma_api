#ifndef ORDERITEMMODEL_H
#define ORDERITEMMODEL_H

#include <string>
#include <iostream>
#include "db/DatabaseConnection.h"
#include "entities/OrderProduct.h"
#include <optional>
#include <vector>
#include <memory>
#include <cstring>

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
    std::optional<int> createOrderItem(const std::vector<OrderProduct> &products, int order_id);
};

#endif