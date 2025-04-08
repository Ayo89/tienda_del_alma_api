#ifndef PRODUCTCONTROLLER_H
#define PRODUCTCONTROLLER_H
#include <cpprest/http_msg.h>
#include <optional>
#include <vector>
#include <iostream>
#include <string>
#include "db/DatabaseConnection.h"
#include "entities/User.h"
#include "model/ProductModel.h"

class ProductController
{
private:
    ProductModel model;

public:
    ProductController(DatabaseConnection &db) : model(db) {} // Pasamos db al constructor de UserModel

    web::http::http_response getAllProducts();
};

#endif