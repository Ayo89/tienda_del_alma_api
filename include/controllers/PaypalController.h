#ifndef PAYPALCONTROLLER_H
#define PAYPALCONTROLLER_H
#include "services/PaypalService.h"
#include "db/DatabaseConnection.h"
#include "AuthUtils.h"
#include "model/OrderModel.h"




class PaypalController
{
public:
    PaypalController();
    web::http::http_response createPayment(const web::http::http_request &request);
};

#endif // PAYPALCONTROLLER_H