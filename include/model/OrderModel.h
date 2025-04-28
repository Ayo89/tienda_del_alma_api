#ifndef ORDERMODEL_H
#define ORDERMODEL_H

#include <optional>
#include <string>
#include <iostream>
#include <memory>
#include <cstring>
#include <vector>
#include "entities/Order.h"
#include "entities/OrderItem.h"
#include "model/OrderItemModel.h"
#include "db/DatabaseConnection.h"
#include "utils/Errors.h"

class OrderModel
{

public:
    OrderModel();

    std::optional<int> createOrder(
        const int &user_id,
        const int &shipping_address_id,
        const int &billing_address_id,
        const std::string &status,
        const std::vector<OrderItem> &products,
        const std::string &shipment_date,
        const std::string &delivery_date,
        const std::string &carrier,
        const std::string &tracking_url,
        const std::string &tracking_number,
        const std::string &payment_method,
        const std::string &payment_status);

    std::optional<std::vector<Order>> getOrdersByUserId(int &user_id);

    std::optional<Order> getPendingOrderByUserId(int &user_id);

    std::optional<Order> getOrderById(int &order_id, int &user_id);

    std::pair<std::optional<Order>, Errors> updateOrder(
        const int &user_id,
        const int &order_id,
        const int &shipping_address_id,
        const int &billing_address_id,
        const std::vector<OrderItem> &products,
        const std::string &status,
        const std::string &shipment_date,
        const std::string &delivery_date,
        const std::string &carrier,
        const std::string &tracking_url,
        const std::string &tracking_number,
        const std::string &payment_method,
        const std::string &payment_status);
};

#endif
