#ifndef PAYPALSERVICE_H
#define PAYPALSERVICE_H
#include "utils/UtilsOwner.h"
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include "PaypalService.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <utility>
#include "db/DatabaseConnection.h"

using namespace web;
using namespace web::http;
using namespace web::http::client;

class PaypalService
{
public:
    PaypalService();
    http_response createPayment(const std::string &total);
    std::string getAccessToken();
    http_response capturePayment(const std::string &orderId);
};

#endif