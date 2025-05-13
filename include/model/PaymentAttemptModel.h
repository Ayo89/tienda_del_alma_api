#ifndef PAYMENTATTEMPTMODEL_H
#define PAYMENTATTEMPTMODEL_H

#include "db/DatabaseConnection.h"
#include "entities/PaymentAttempt.h"
#include "utils/Errors.h"
#include <optional>
#include <memory>
#include <iostream>
#include <cstring>

class PaymentAttempModel
{
public:
    PaymentAttempModel() {};
    std::pair<std::optional<PaymentAttempt>, Errors> createPaymentAttempt(int user_id, int order_id, std::string cart_hash, double total, std::string idempotency_key, std::string paypal_order_id, std::string status);
};

#endif