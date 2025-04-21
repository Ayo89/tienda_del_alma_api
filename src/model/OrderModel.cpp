#include "model/OrderModel.h"
#include "db/DatabaseConnection.h"
#include <iostream>
#include <mysql/mysql.h>

OrderModel::OrderModel() = default;

std::optional<int> OrderModel::createOrder(
    const int &user_id,
    const int &shipping_address_id,
    const int &billing_address_id,
    const std::string &status,
    const double &total,
    const std::vector<OrderProduct> &products,
    const std::string &shipment_date,
    const std::string &delivery_date,
    const std::string &carrier,
    const std::string &tracking_url,
    const std::string &tracking_number,
    const std::string &payment_method,
    const std::string &payment_status)
{
    // Validación básica
    if (shipping_address_id == 0 || billing_address_id == 0)
    {
        std::cerr << "Error: Required fields cannot be empty" << std::endl;
        return std::nullopt;
    }

    // Obtener conexión
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0)
    {
        std::cerr << "Error: No active database connection: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    // Iniciar transacción
    if (mysql_query(conn, "START TRANSACTION") != 0)
    {
        std::cerr << "Error starting transaction: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    // Preparar INSERT para la cabecera
    const char *sql =
        "INSERT INTO orders (user_id, shipping_address_id, billing_address_id, status, total, "
        "shipment_date, delivery_date, carrier, tracking_url, tracking_number, payment_method, payment_status) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Statement initialization failed: " << mysql_error(conn) << std::endl;
        mysql_query(conn, "ROLLBACK");
        return std::nullopt;
    }
    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);

    if (mysql_stmt_prepare(stmt, sql, strlen(sql)) != 0)
    {
        std::cerr << "Statement preparation failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_query(conn, "ROLLBACK");
        return std::nullopt;
    }

    // Configurar bindings
    MYSQL_BIND bind[12];
    memset(bind, 0, sizeof(bind));

    // Variables para longitudes
    unsigned long len_status = status.size();
    unsigned long len_ship_date = shipment_date.size();
    unsigned long len_deliv_date = delivery_date.size();
    unsigned long len_carrier = carrier.size();
    unsigned long len_track_url = tracking_url.size();
    unsigned long len_track_number = tracking_number.size();
    unsigned long len_pay_method = payment_method.size();
    unsigned long len_pay_status = payment_status.size();

    // 0) user_id
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (void *)&user_id;

    // 1) shipping_address_id
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = (void *)&shipping_address_id;

    // 2) billing_address_id
    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer = (void *)&billing_address_id;

    // 3) status
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (void *)status.c_str();
    bind[3].buffer_length = len_status;
    bind[3].length = &len_status;

    // 4) total
    double total_val = total;
    bind[4].buffer_type = MYSQL_TYPE_DOUBLE;
    bind[4].buffer = (void *)&total_val;
    bind[4].is_null = 0;

    // 5) shipment_date
    bind[5].buffer_type = MYSQL_TYPE_STRING;
    bind[5].buffer = (void *)shipment_date.c_str();
    bind[5].buffer_length = len_ship_date;
    bind[5].length = &len_ship_date;

    // 6) delivery_date
    bind[6].buffer_type = MYSQL_TYPE_STRING;
    bind[6].buffer = (void *)delivery_date.c_str();
    bind[6].buffer_length = len_deliv_date;
    bind[6].length = &len_deliv_date;

    // 7) carrier
    bind[7].buffer_type = MYSQL_TYPE_STRING;
    bind[7].buffer = (void *)carrier.c_str();
    bind[7].buffer_length = len_carrier;
    bind[7].length = &len_carrier;

    // 8) tracking_url
    bind[8].buffer_type = MYSQL_TYPE_STRING;
    bind[8].buffer = (void *)tracking_url.c_str();
    bind[8].buffer_length = len_track_url;
    bind[8].length = &len_track_url;

    // 9) tracking_number
    bind[9].buffer_type = MYSQL_TYPE_STRING;
    bind[9].buffer = (void *)tracking_number.c_str();
    bind[9].buffer_length = len_track_number;
    bind[9].length = &len_track_number;

    // 10) payment_method
    bind[10].buffer_type = MYSQL_TYPE_STRING;
    bind[10].buffer = (void *)payment_method.c_str();
    bind[10].buffer_length = len_pay_method;
    bind[10].length = &len_pay_method;

    // 11) payment_status
    bind[11].buffer_type = MYSQL_TYPE_STRING;
    bind[11].buffer = (void *)payment_status.c_str();
    bind[11].buffer_length = len_pay_status;
    bind[11].length = &len_pay_status;

    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        std::cerr << "Parameter binding failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_query(conn, "ROLLBACK");
        return std::nullopt;
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Statement execution failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_query(conn, "ROLLBACK");
        return std::nullopt;
    }

    int order_id = mysql_insert_id(conn);
    if (order_id == 0)
    {
        std::cerr << "Error retrieving last insert ID: " << mysql_error(conn) << std::endl;
        mysql_query(conn, "ROLLBACK");
        return std::nullopt;
    }

    // Insertar cada línea de pedido
    for (const auto &item : products)
    {
        std::string insertItem =
            "INSERT INTO order_items (order_id, product_id, quantity, price) VALUES (" +
            std::to_string(order_id) + ", " +
            std::to_string(item.product_id) + ", " +
            std::to_string(item.quantity) + ", " +
            std::to_string(item.price) + ");";
        if (mysql_query(conn, insertItem.c_str()) != 0)
        {
            std::cerr << "Error inserting order_item: " << mysql_error(conn) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return std::nullopt;
        }
    }

    // Confirmar transacción
    if (mysql_query(conn, "COMMIT") != 0)
    {
        std::cerr << "Commit failed: " << mysql_error(conn) << std::endl;
        mysql_query(conn, "ROLLBACK");
        return std::nullopt;
    }

    return order_id;
}
