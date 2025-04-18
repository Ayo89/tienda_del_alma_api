#include "controllers/ProductController.h"
#include <cpprest/json.h>
#include <iostream>

using namespace web;
using namespace web::http;

ProductController::ProductController() : model() {} // Inicializamos el modelo en el constructor

http_response ProductController::getAllProducts()
{
    http_response response;
    try
    {
        // Call model.getAllProducts(), que ahora usa el Singleton internamente
        auto products_opt = model.getAllProducts();

        // El resto de tu lógica para manejar la respuesta permanece igual
        if (!products_opt.has_value())
        {
            response.set_status_code(status_codes::InternalError);
            response.set_body(json::value::object({{U("message"), json::value::string(U("Error al obtener productos"))}}));
            std::cout << "Error al obtener productos, devolviendo 500" << std::endl;
        }
        else
        {
            const auto &products = products_opt.value();
            std::cout << "Productos obtenidos: " << products.size() << std::endl;

            if (products.empty())
            {
                response.set_status_code(status_codes::OK);
                response.set_body(json::value::array());
                std::cout << "Sin productos, devolviendo 200 con array vacío" << std::endl;
            }
            else
            {
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
                response.set_status_code(status_codes::OK);
                response.set_body(result);
            }
        }
    }
    catch (const std::exception &e)
    {
        response.set_status_code(status_codes::InternalError);
        response.set_body(json::value::object({{U("message"), json::value::string(U("Error inesperado al obtener productos"))}}));
        std::cout << "Excepción en getAllProducts: " << e.what() << std::endl;
    }
    response.headers().set_content_type(U("application/json"));
    return response;
}