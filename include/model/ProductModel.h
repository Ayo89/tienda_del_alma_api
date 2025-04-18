#ifndef PRODUCTMODEL_H
#define PRODUCTMODEL_H

#include <optional>
#include <string>
#include <vector>
#include "entities/Product.h"
#include "db/DatabaseConnection.h"
#include "db/DatabaseInitializer.h"

class ProductModel
{

public:
    ProductModel();
    bool insertSampleProducts();
    std::optional<std::vector<Product>> getAllProducts();
};

#endif // PRODUCTMODEL_H
