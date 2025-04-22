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
    // Basic validation
    if (shipping_address_id == 0 || billing_address_id == 0)
    {
        std::cerr << "Error: Required fields cannot be empty" << std::endl;
        return std::nullopt;
    }

    // Get database connection
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0)
    {
        std::cerr << "Error: No active database connection: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    // Start transaction
    if (mysql_query(conn, "START TRANSACTION") != 0)
    {
        std::cerr << "Error starting transaction: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    // Prepare the SQL statement
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

    // Bind the parameters
    MYSQL_BIND bind[12];
    memset(bind, 0, sizeof(bind));

    // Set the parameter types and values
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

    // Insert order items
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

    // Commit the transaction
    if (mysql_query(conn, "COMMIT") != 0)
    {
        std::cerr << "Commit failed: " << mysql_error(conn) << std::endl;
        mysql_query(conn, "ROLLBACK");
        return std::nullopt;
    }

    return order_id;
}

std::optional<std::vector<Order>> OrderModel::getOrdersByUserId(int &user_id)
{
    auto &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "No DB connection\n";
        return std::nullopt;
    }

    const char *query =
        "SELECT id, user_id, shipping_address_id, billing_address_id,"
        "       order_date, status, total, shipment_date,"
        "       delivery_date, carrier, tracking_url,"
        "       tracking_number, payment_method, payment_status"
        "  FROM orders WHERE user_id = ?";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Init stmt failed: " << mysql_error(conn) << "\n";
        return std::nullopt;
    }
    std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>
        stmt_guard(stmt, mysql_stmt_close);

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cerr << "Prepare failed: " << mysql_stmt_error(stmt) << "\n";
        return std::nullopt;
    }

    // Bind del parámetro user_id
    MYSQL_BIND param{};
    param.buffer_type = MYSQL_TYPE_LONG;
    param.buffer = &user_id;
    param.buffer_length = sizeof(user_id);
    if (mysql_stmt_bind_param(stmt, &param) != 0)
    {
        std::cerr << "Bind param failed: " << mysql_stmt_error(stmt) << "\n";
        return std::nullopt;
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Execute failed: " << mysql_stmt_error(stmt) << "\n";
        return std::nullopt;
    }

    if (mysql_stmt_store_result(stmt) != 0)
    {
        std::cerr << "Store result failed: " << mysql_stmt_error(stmt) << "\n";
        return std::nullopt;
    }

    // Buffers para cada columna
    int id, shipping_address_id, billing_address_id;
    char order_date[64], status[32], total_str[32];
    char ship_date[64], delivery_date[64], carrier[64];
    char tracking_url[128], tracking_num[64];
    char pay_method[32], pay_status[32];

    unsigned long length[14];
    bool is_null[14];

    MYSQL_BIND result_bind[14];
    memset(result_bind, 0, sizeof(result_bind));

    // Columna 0 → id
    result_bind[0].buffer_type = MYSQL_TYPE_LONG;
    result_bind[0].buffer = &id;
    result_bind[0].buffer_length = sizeof(id);
    result_bind[0].is_null = &is_null[0];
    result_bind[0].length = &length[0];

    // Columna 1 → user_id
    result_bind[1].buffer_type = MYSQL_TYPE_LONG;
    result_bind[1].buffer = &user_id;
    result_bind[1].buffer_length = sizeof(user_id);
    result_bind[1].is_null = &is_null[1];
    result_bind[1].length = &length[1];

    // Columna 2 → shipping_address_id
    result_bind[2].buffer_type = MYSQL_TYPE_LONG;
    result_bind[2].buffer = &shipping_address_id;
    result_bind[2].buffer_length = sizeof(shipping_address_id);
    result_bind[2].is_null = &is_null[2];
    result_bind[2].length = &length[2];

    // Columna 3 → billing_address_id
    result_bind[3].buffer_type = MYSQL_TYPE_LONG;
    result_bind[3].buffer = &billing_address_id;
    result_bind[3].buffer_length = sizeof(billing_address_id);
    result_bind[3].is_null = &is_null[3];
    result_bind[3].length = &length[3];

    // Columna 4 → order_date
    result_bind[4].buffer_type = MYSQL_TYPE_STRING;
    result_bind[4].buffer = order_date;
    result_bind[4].buffer_length = sizeof(order_date);
    result_bind[4].is_null = &is_null[4];
    result_bind[4].length = &length[4];

    // Columna 5 → status
    result_bind[5].buffer_type = MYSQL_TYPE_STRING;
    result_bind[5].buffer = status;
    result_bind[5].buffer_length = sizeof(status);
    result_bind[5].is_null = &is_null[5];
    result_bind[5].length = &length[5];

    // Columna 6 → total
    result_bind[6].buffer_type = MYSQL_TYPE_STRING;
    result_bind[6].buffer = total_str;
    result_bind[6].buffer_length = sizeof(total_str);
    result_bind[6].is_null = &is_null[6];
    result_bind[6].length = &length[6];

    // Columna 7 → ship_date
    result_bind[7].buffer_type = MYSQL_TYPE_STRING;
    result_bind[7].buffer = ship_date;
    result_bind[7].buffer_length = sizeof(ship_date);
    result_bind[7].is_null = &is_null[7];
    result_bind[7].length = &length[7];

    // Columna 8 → delivery_date
    result_bind[8].buffer_type = MYSQL_TYPE_STRING;
    result_bind[8].buffer = delivery_date;
    result_bind[8].buffer_length = sizeof(delivery_date);
    result_bind[8].is_null = &is_null[8];
    result_bind[8].length = &length[8];

    // Columna 9 → carrier
    result_bind[9].buffer_type = MYSQL_TYPE_STRING;
    result_bind[9].buffer = carrier;
    result_bind[9].buffer_length = sizeof(carrier);
    result_bind[9].is_null = &is_null[9];    
    result_bind[9].length = &length[9];

    // Columna 10 → tracking_url
    result_bind[10].buffer_type = MYSQL_TYPE_STRING;
    result_bind[10].buffer = tracking_url;
    result_bind[10].buffer_length = sizeof(tracking_url);
    result_bind[10].is_null = &is_null[10];
    result_bind[10].length = &length[10];

    // Columna 11 → tracking_number
    result_bind[11].buffer_type = MYSQL_TYPE_STRING;
    result_bind[11].buffer = tracking_num;
    result_bind[11].buffer_length = sizeof(tracking_num);
    result_bind[11].is_null = &is_null[11];
    result_bind[11].length = &length[11];

    // Columna 12 → payment_method
    result_bind[12].buffer_type = MYSQL_TYPE_STRING;
    result_bind[12].buffer = pay_method;
    result_bind[12].buffer_length = sizeof(pay_method);
    result_bind[12].is_null = &is_null[12];
    result_bind[12].length = &length[12];

    // Columna 13 → payment_status
    result_bind[13].buffer_type = MYSQL_TYPE_STRING;
    result_bind[13].buffer = pay_status;
    result_bind[13].buffer_length = sizeof(pay_status);
    result_bind[13].is_null = &is_null[13];
    result_bind[13].length = &length[13];
    


    if (mysql_stmt_bind_result(stmt, result_bind) != 0)
    {
        std::cerr << "Bind result failed: " << mysql_stmt_error(stmt) << "\n";
        return std::nullopt;
    }

    std::vector<Order> orders;
    while (mysql_stmt_fetch(stmt) == 0)
    {
        Order ord;
        ord.id = id;
        ord.user_id = user_id;
        ord.shipping_address_id = shipping_address_id;
        ord.billing_address_id = billing_address_id;
        ord.order_date = order_date;
        ord.status = status;
        ord.total = std::stod(total_str);
        ord.shipment_date = ship_date;
        ord.delivery_date = delivery_date;
        ord.carrier = carrier;
        ord.tracking_url = tracking_url;
        ord.tracking_number = tracking_num;
        ord.payment_method = pay_method;
        ord.payment_status = pay_status;
        orders.push_back(ord);
    }

    return orders;
}
