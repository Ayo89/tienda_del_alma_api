#ifndef ORDER_H
#define ORDER_H

#include <string>

struct Order
{
    int id;
    int user_id;
    int shipping_address_id;
    int billing_address_id;
    std::string order_date;
    std::string status;
    double total;
    std::string shipment_date;
    std::string delivery_date;
    std::string carrier;
    std::string tracking_url;
    std::string tracking_number;
    std::string payment_method;
    std::string payment_status;

};

#endif
