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
private:
    DatabaseConnection &db; // Referencia almacenada

public:
    ProductModel(DatabaseConnection &database) : db(database) {}
    bool insertSampleProducts();
    std::optional<std::vector<Product>> getAllProducts();
};

#endif // PRODUCTMODEL_H
