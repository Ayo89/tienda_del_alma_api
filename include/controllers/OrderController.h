#ifndef ORDERCONTROLLER_H
#define ORDERCONTROLLER_H

#include <cpprest/http_msg.h>
#include <cpprest/json.h>
#include <optional>
#include <string>
#include "db/DatabaseConnection.h"
#include "entities/Order.h"
#include "model/OrderModel.h"
#include "model/CarrierModel.h"
#include "AuthUtils.h"
#include "services/jwt/JwtService.h"
#include "utils/Errors.h"

class OrderController
{
private:
    OrderModel model;

public:
    OrderController();
    web::http::http_response createOrder(const web::http::http_request &request);
    web::http::http_response getOrdersByUserId(const web::http::http_request &request);
    web::http::http_response getOrderById(const web::http::http_request &request);
    web::http::http_response updateTotalbyOrderId(const web::http::http_request &request);
};

#endif