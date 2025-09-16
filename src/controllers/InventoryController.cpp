#include "controllers/InventoryController.h"
#include <iostream>

using namespace web;
using namespace web::http;

InventoryController::InventoryController() {}

http_response InventoryController::getInventoryByProductId(const http_request &request, int productId) {
    http_response response;

    // Usamos el modelo para obtener todos los items (puedes optimizar con un método dedicado si quieres)
    auto items = model.getAllInventory();

    // Buscar el producto específico en el vector
    auto it = std::find_if(items.begin(), items.end(), [productId](const InventoryItem &item) {
        return item.productId == productId;
    });

    if (it == items.end()) {
        response.set_status_code(status_codes::NotFound);
        response.set_body(json::value::object({
            { U("message"), json::value::string(U("Producto no encontrado en inventario")) }
        }));
    } else {
        json::value body;
        body[U("productId")] = json::value::number(it->productId);
        body[U("sku")] = json::value::string(utility::conversions::to_string_t(it->sku));
        body[U("name")] = json::value::string(utility::conversions::to_string_t(it->name));
        body[U("quantity")] = json::value::number(it->quantity);

        response.set_status_code(status_codes::OK);
        response.set_body(body);
    }

    response.headers().set_content_type(U("application/json"));
    return response;
}

http_response InventoryController::updateQuantityByProductId(const http_request &request, int productId) {
    http_response response;
    try {
        auto bodyJson = request.extract_json().get();
        if (!bodyJson.has_field(U("quantity"))) {
            response.set_status_code(status_codes::BadRequest);
            response.set_body(json::value::object({
                { U("message"), json::value::string(U("Falta el campo 'quantity' en el body")) }
            }));
            return response;
        }

        int newQuantity = bodyJson.at(U("quantity")).as_integer();
        bool success = model.updateQuantity(productId, newQuantity);

        if (!success) {
            response.set_status_code(status_codes::InternalError);
            response.set_body(json::value::object({
                { U("message"), json::value::string(U("No se pudo actualizar el inventario")) }
            }));
        } else {
            response.set_status_code(status_codes::OK);
            response.set_body(json::value::object({
                { U("message"), json::value::string(U("Cantidad actualizada correctamente")) },
                { U("productId"), json::value::number(productId) },
                { U("quantity"), json::value::number(newQuantity) }
            }));
        }
    } catch (const std::exception &e) {
        response.set_status_code(status_codes::BadRequest);
        response.set_body(json::value::object({
            { U("message"), json::value::string(U("Error procesando la petición")) }
        }));
        std::cerr << "Error en updateQuantityByProductId: " << e.what() << std::endl;
    }

    response.headers().set_content_type(U("application/json"));
    return response;
}

http_response InventoryController::getAllInventory(const http_request &request) {
    http_response response;

    auto items = model.getAllInventory();
    json::value body = json::value::array();

    for (size_t i = 0; i < items.size(); ++i) {
        const auto &item = items[i];
        json::value jsonItem;
        jsonItem[U("productId")] = json::value::number(item.productId);
        jsonItem[U("sku")] = json::value::string(utility::conversions::to_string_t(item.sku));
        jsonItem[U("name")] = json::value::string(utility::conversions::to_string_t(item.name));
        jsonItem[U("quantity")] = json::value::number(item.quantity);
        body[i] = jsonItem;
    }

    response.set_status_code(status_codes::OK);
    response.set_body(body);
    response.headers().set_content_type(U("application/json"));
    return response;
}
