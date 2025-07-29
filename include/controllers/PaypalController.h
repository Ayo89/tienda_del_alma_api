#ifndef PAYPALCONTROLLER_H
#define PAYPALCONTROLLER_H
#include "services/PaypalService.h"
#include "db/DatabaseConnection.h"
#include "AuthUtils.h"
#include "model/OrderModel.h"
#include "model/PaymentAttemptModel.h"



class PaypalController
{
public:
    PaypalController();
    web::http::http_response createPayment(const web::http::http_request &request, const int user_id);
    web::http::http_response capturePayment(const web::http::http_request &request, const int user_id);
};

#endif // PAYPALCONTROLLER_H