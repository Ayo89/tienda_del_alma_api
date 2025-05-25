#include "controllers/CategoryController.h"

web::http::http_response CategoryController::getAllCategories()
{

    web::http::http_response response;

    CategoryModel categoryModel;
    auto [optCategories, errors] = categoryModel.getAllCategories();

    if (!optCategories.has_value())
    {
        response.set_status_code(web::http::status_codes::NotFound);
        response.set_body(U("No categories found"));
        return response;
    }

    response.set_status_code(web::http::status_codes::OK);

    web::json::value json_response = web::json::value::array();

    for (size_t i = 0; i < optCategories->size(); ++i)
    {
        const auto &category = optCategories->at(i);
        json_response[i] = web::json::value::object();
        json_response[i][U("id")] = web::json::value::number(category.id);
        json_response[i][U("name")] = web::json::value::string(category.name);
    }

    response.set_body(json_response);

    return response;
}
