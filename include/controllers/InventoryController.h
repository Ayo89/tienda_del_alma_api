#ifndef INVENTORYCONTROLLER_H
#define INVENTORYCONTROLLER_H

#include <cpprest/http_msg.h>
#include <cpprest/json.h>
#include "model/InventoryModel.h"

class InventoryController {
private:
    InventoryModel model;

public:
    InventoryController();

    // Devuelve inventario de un solo producto (id, sku, name, quantity)
    web::http::http_response getInventoryByProductId(const web::http::http_request &request, int productId);

    // Actualiza cantidad
    web::http::http_response updateQuantityByProductId(const web::http::http_request &request, int productId);

    // Devuelve todos los productos con inventario
    web::http::http_response getAllInventory(const web::http::http_request &request);
};

#endif
