#pragma once

#include <string>

struct InventoryItem {
    int productId;
    std::string sku;
    std::string name;
    int quantity;
};