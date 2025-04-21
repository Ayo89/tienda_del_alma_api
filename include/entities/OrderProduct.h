#ifndef ORDERPRODUCT_H
#define ORDERPRODUCT_H

#include <string>

struct OrderProduct
{
    int id;
    int order_id;
    int product_id;
    int quantity;
    double price;
};

#endif
