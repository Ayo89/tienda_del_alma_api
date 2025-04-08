#include "controllers/ProductController.h"
#include <cpprest/json.h>

using namespace web;
using namespace web::http;

http_response ProductController::getAllProducts()
{
    http_response response;
    auto products = model.getAllProducts();
    std::cout << "Productos obtenidos: " << products.size() << std::endl;

    if (products.empty())
    {
        response.set_status_code(status_codes::NotFound);
        response.set_body(json::value::object({{U("message"), json::value::string(U("No se encontraron productos"))}}));
        std::cout << "Sin productos, devolviendo 404" << std::endl;
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
            jsonProduct[U("image_url")] = json::value::string(utility::conversions::to_string_t(product.imageUrl));
            jsonProduct[U("category_id")] = json::value::number(product.categoryId);
            result[i] = jsonProduct;
        }
        response.set_status_code(status_codes::OK);
        response.set_body(result);
    }
    response.headers().set_content_type(U("application/json"));
    return response;
}

#include "controllers/ProductController.h"
#include <cpprest/json.h>

using namespace web;
using namespace web::http;
