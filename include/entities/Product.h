#ifndef PRODUCT_H
#define PRODUCT_H

#include <string>

struct Product
{
    int id;
    std::string sku;
    std::string name;
    std::string description;
    double price;
    std::string imageUrl;
    int categoryId;
};

#endif
