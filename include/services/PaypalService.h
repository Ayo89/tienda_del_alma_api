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

using namespace web;
using namespace web::http;
using namespace web::http::client;

class PaypalService
{
public:
    PaypalService();
    http_response createPayment(const double &total);
    std::string getAccessToken();
};

#endif