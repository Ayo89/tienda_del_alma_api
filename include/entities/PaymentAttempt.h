#ifndef PAYMENTATTEMP_H
#define PAYMENTATTEMP_H

#include <string>

struct PaymentAttempt
{
    int id;
    int user_id;
    int order_id;
    std::string cart_hash;
    double total;
    std::string idempotency_key;
    std::string paypal_order_id;
    std::string status;
    std::string created_at;
};

#endif
