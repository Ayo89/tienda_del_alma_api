#pragma once
#include <optional>
#include <vector>
#include "entities/inventory.h"
#include "entities/OrderItem.h"
#include "utils/Errors.h"

class InventoryModel
{
public:
    InventoryModel();

    // Devuelve la cantidad de stock de un producto por su ID
    std::optional<int> getQuantityByProductId(int productId);

    // Actualiza la cantidad de stock de un producto
    bool updateQuantity(int productId, int newQuantity);

    // Devuelve todos los productos con su inventario (id, sku, nombre, cantidad)
    std::vector<InventoryItem> getAllInventory();

    std::pair<bool, Errors> reserveStockForItems(const std::vector<OrderItem> &items);
    std::pair<bool, Errors> releaseStockForItems(const std::vector<OrderItem> &items);
};
