#ifndef ORDERMODEL_H
#define ORDERMODEL_H

#include <optional>
#include <string>
#include <iostream>
#include <memory>
#include <cstring>
#include <vector>
#include "entities/Order.h"
#include "entities/OrderProduct.h"
#include "db/DatabaseConnection.h"
#include "db/DatabaseInitializer.h"

class OrderModel
{

public:
    OrderModel();

    std::optional<int> createOrder(
        const int &user_id,
        const int &shipping_address_id,
        const int &billing_address_id,
        const std::string &status,
        const double &total,
        const std::vector<OrderProduct> &products,
        const std::string &shipment_date,
        const std::string &delivery_date,
        const std::string &carrier,
        const std::string &tracking_url,
        const std::string &tracking_number,
        const std::string &payment_method,
        const std::string &payment_status);
};

#endif
