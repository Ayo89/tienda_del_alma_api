#include "model/InventoryModel.h"
#include "db/DatabaseConnection.h"
#include <mysql/mysql.h>
#include <iostream>
#include <cstring>

InventoryModel::InventoryModel() {}

std::optional<int> InventoryModel::getQuantityByProductId(int productId) {
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0) {
        std::cerr << "Error: No active DB connection: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    const char *query = "SELECT quantity FROM inventory WHERE product_id = ?";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt) {
        std::cerr << "Statement init failed: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
        std::cerr << "Prepare failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    MYSQL_BIND param{};
    param.buffer_type = MYSQL_TYPE_LONG;
    param.buffer = (void*)&productId;

    if (mysql_stmt_bind_param(stmt, &param) != 0) {
        std::cerr << "Bind param failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    if (mysql_stmt_execute(stmt) != 0) {
        std::cerr << "Execute failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    int quantity = 0;
    MYSQL_BIND result{};
    result.buffer_type = MYSQL_TYPE_LONG;
    result.buffer = (void*)&quantity;

    if (mysql_stmt_bind_result(stmt, &result) != 0) {
        std::cerr << "Bind result failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    if (mysql_stmt_fetch(stmt) == 0) {
        mysql_stmt_close(stmt);
        return quantity;
    }

    mysql_stmt_close(stmt);
    return std::nullopt;
}

bool InventoryModel::updateQuantity(int productId, int newQuantity) {
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0) {
        std::cerr << "Error: No active DB connection: " << mysql_error(conn) << std::endl;
        return false;
    }

    const char *query = "UPDATE inventory SET quantity = ? WHERE product_id = ?";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt) {
        std::cerr << "Statement init failed: " << mysql_error(conn) << std::endl;
        return false;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
        std::cerr << "Prepare failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }

    int data[2] = {newQuantity, productId};
    MYSQL_BIND params[2]{};
    memset(params, 0, sizeof(params));

    params[0].buffer_type = MYSQL_TYPE_LONG;
    params[0].buffer = (void*)&data[0];

    params[1].buffer_type = MYSQL_TYPE_LONG;
    params[1].buffer = (void*)&data[1];

    if (mysql_stmt_bind_param(stmt, params) != 0) {
        std::cerr << "Bind params failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }

    bool success = (mysql_stmt_execute(stmt) == 0);
    if (!success) {
        std::cerr << "Execution failed: " << mysql_stmt_error(stmt) << std::endl;
    }

    mysql_stmt_close(stmt);
    return success;
}

std::vector<InventoryItem> InventoryModel::getAllInventory() {
    std::vector<InventoryItem> results;

    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0) {
        std::cerr << "Error: No active DB connection: " << mysql_error(conn) << std::endl;
        return results;
    }

    const char *query =
        "SELECT p.id, p.sku, p.name, i.quantity "
        "FROM inventory i "
        "INNER JOIN products p ON i.product_id = p.id";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt) {
        std::cerr << "Statement init failed: " << mysql_error(conn) << std::endl;
        return results;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
        std::cerr << "Prepare failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return results;
    }

    if (mysql_stmt_execute(stmt) != 0) {
        std::cerr << "Execute failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return results;
    }

    int productId = 0;
    char sku[50] = {0};
    char name[100] = {0};
    int quantity = 0;

    unsigned long len[3];
    bool is_null[3];
    bool error[3];

    MYSQL_BIND result[4];
    memset(result, 0, sizeof(result));

    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = (void *)&productId;

    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = (void *)sku;
    result[1].buffer_length = sizeof(sku);
    result[1].length = &len[1];
    result[1].is_null = &is_null[1];
    result[1].error = &error[1];

    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = (void *)name;
    result[2].buffer_length = sizeof(name);
    result[2].length = &len[2];
    result[2].is_null = &is_null[2];
    result[2].error = &error[2];

    result[3].buffer_type = MYSQL_TYPE_LONG;
    result[3].buffer = (void *)&quantity;

    if (mysql_stmt_bind_result(stmt, result) != 0) {
        std::cerr << "Bind result failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return results;
    }

    while (mysql_stmt_fetch(stmt) == 0) {
        InventoryItem item;
        item.productId = productId;
        item.sku = std::string(sku, len[1]);
        item.name = std::string(name, len[2]);
        item.quantity = quantity;
        results.push_back(item);
    }

    mysql_stmt_close(stmt);
    return results;
}