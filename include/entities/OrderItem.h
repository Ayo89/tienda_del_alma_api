#ifndef ORDERITEM_H
#define ORDERITEM_H

#include <string>

struct OrderItem
{
    int id;
    int order_id;
    int product_id;
    int quantity;
    double price;
};

#endif
