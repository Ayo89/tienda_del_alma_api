#ifndef CATEGORYCONTROLLER_H
#define CATEGORYCONTROLLER_H
#include <cpprest/http_msg.h>
#include <cpprest/json.h>
#include <optional>
#include <vector>
#include "model/CategoryModel.h" // Incluimos el modelo

class CategoryController
{
private:
    CategoryModel model;

public:
    CategoryController() {};
    web::http::http_response getAllCategories();
};

#endif