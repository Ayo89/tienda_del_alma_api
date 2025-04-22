#ifndef PRODUCTCONTROLLER_H
#define PRODUCTCONTROLLER_H

#include <cpprest/http_msg.h>
#include <cpprest/json.h>
#include <optional>
#include <vector>
#include <iostream>
#include <string>
#include "model/ProductModel.h" // Incluimos el modelo

class ProductController
{
private:
    ProductModel model;

public:
    ProductController();
    web::http::http_response getAllProducts();
};

#endif