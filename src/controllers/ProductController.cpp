#include "controllers/ProductController.h"
#include <cpprest/json.h>
#include <iostream>

using namespace web;
using namespace web::http;

http_response ProductController::getAllProducts()
{
    http_response response;
    try
    {
        // Call model.getAllProducts(), which returns std::optional<std::vector<Product>>
        auto products_opt = model.getAllProducts();

        // Check if the optional contains a value
        if (!products_opt.has_value())
        {
            // No value means an error occurred (e.g., database connection failed)
            response.set_status_code(status_codes::InternalError); // 500
            response.set_body(json::value::object({{U("message"), json::value::string(U("Error al obtener productos"))}}));
            std::cout << "Error al obtener productos, devolviendo 500" << std::endl;
        }
        else
        {
            // Extract the vector from the optional
            const auto &products = products_opt.value();
            std::cout << "Productos obtenidos: " << products.size() << std::endl;

            if (products.empty())
            {
                // Return 200 OK with an empty array if no products exist
                response.set_status_code(status_codes::OK); // 200
                response.set_body(json::value::array());
                std::cout << "Sin productos, devolviendo 200 con array vacío" << std::endl;
            }
            else
            {
                // Build the JSON array of products
                json::value result = json::value::array();
                for (size_t i = 0; i < products.size(); ++i)
                {
                    const auto &product = products[i];
                    json::value jsonProduct;
                    jsonProduct[U("id")] = json::value::number(product.id);
                    jsonProduct[U("sku")] = json::value::string(utility::conversions::to_string_t(product.sku));
                    jsonProduct[U("name")] = json::value::string(utility::conversions::to_string_t(product.name));
                    jsonProduct[U("description")] = json::value::string(utility::conversions::to_string_t(product.description));
                    jsonProduct[U("price")] = json::value::number(product.price);
                    jsonProduct[U("image_url")] = json::value::string(utility::conversions::to_string_t(product.image_url));
                    jsonProduct[U("category_id")] = json::value::number(product.category_id);
                    result[i] = jsonProduct;
                }
                response.set_status_code(status_codes::OK); // 200
                response.set_body(result);
            }
        }
    }
    catch (const std::exception &e)
    {
        // Handle unexpected exceptions
        response.set_status_code(status_codes::InternalError); // 500
        response.set_body(json::value::object({{U("message"), json::value::string(U("Error inesperado al obtener productos"))}}));
        std::cout << "Excepción en getAllProducts: " << e.what() << std::endl;
    }
    response.headers().set_content_type(U("application/json"));
    return response;
}