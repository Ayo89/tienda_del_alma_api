#include "model/OrderModel.h"

OrderModel::OrderModel() = default;
OrderItemModel orderItemModel;

std::optional<int> OrderModel::createOrder(
    const int &user_id,
    const int &shipping_address_id,
    const int &billing_address_id,
    const std::vector<OrderItem> &products,
    const std::string &shipment_date,
    const std::string &delivery_date,
    const int &carrier_id,
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
        std::cerr << "Error starting transaction createOrder: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    // Prepare the SQL statement
    const char *sql =
        "INSERT INTO orders (user_id, shipping_address_id, billing_address_id, status, total, "
        "shipment_date, delivery_date, carrier_id, tracking_url, tracking_number, payment_method, payment_status) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Statement initialization failed createOrder: " << mysql_error(conn) << std::endl;
        mysql_query(conn, "ROLLBACK");
        return std::nullopt;
    }
    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);

    if (mysql_stmt_prepare(stmt, sql, strlen(sql)) != 0)
    {
        std::cerr << "Statement preparation failed createOrder: " << mysql_stmt_error(stmt) << std::endl;
        mysql_query(conn, "ROLLBACK");
        return std::nullopt;
    }

    // Calculate the total amount
    double total = orderItemModel.calculateOrderTotal(products);

    // Bind the parameters
    MYSQL_BIND bind[12];
    memset(bind, 0, sizeof(bind));
    std::string status = "pending"; // Default status

    // Set the parameter types and values
    unsigned long len_status = status.size();
    unsigned long len_ship_date = shipment_date.size();
    unsigned long len_deliv_date = delivery_date.size();
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
    bind[7].buffer_type = MYSQL_TYPE_LONG;
    bind[7].buffer = (void *)&carrier_id;
    bind[7].is_null = 0;

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
        std::cerr << "Parameter binding failed createOrder: " << mysql_stmt_error(stmt) << std::endl;
        mysql_query(conn, "ROLLBACK");
        return std::nullopt;
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Statement execution failed createOrder: " << mysql_stmt_error(stmt) << std::endl;
        mysql_query(conn, "ROLLBACK");
        return std::nullopt;
    }

    int order_id = mysql_insert_id(conn);
    if (order_id == 0)
    {
        std::cerr << "Error retrieving last insert ID in createOrder: " << mysql_error(conn) << std::endl;
        mysql_query(conn, "ROLLBACK");
        return std::nullopt;
    }

    OrderItemModel orderItem;
    if (!orderItem.createOrderItem(products, order_id))
    {
        std::cerr << "Error creating order items: " << mysql_error(conn) << std::endl;
        mysql_query(conn, "ROLLBACK");
        return std::nullopt;
    }

    // Commit the transaction
    if (mysql_query(conn, "COMMIT") != 0)
    {
        std::cerr << "Commit failed createOrder: " << mysql_error(conn) << std::endl;
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
        "       delivery_date, carrier_id, tracking_url,"
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
    int id, user_id_query, shipping_address_id, billing_address_id, carrier_id;
    double total;
    char order_date[64], status[32];
    char ship_date[64], delivery_date[64];
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
    result_bind[1].buffer = &user_id_query;
    result_bind[1].buffer_length = sizeof(user_id_query);
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
    result_bind[6].buffer_type = MYSQL_TYPE_LONG;
    result_bind[6].buffer = &total;
    result_bind[6].buffer_length = sizeof(total);
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
    result_bind[9].buffer_type = MYSQL_TYPE_LONG;
    result_bind[9].buffer = &carrier_id;
    result_bind[9].buffer_length = sizeof(carrier_id);
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

    // Bind the result

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
        ord.user_id = user_id_query;
        ord.shipping_address_id = shipping_address_id;
        ord.billing_address_id = billing_address_id;
        ord.order_date = order_date;
        ord.status = status;
        ord.total = total;
        ord.shipment_date = ship_date;
        ord.delivery_date = delivery_date;
        ord.carrier_id = carrier_id;
        ord.tracking_url = tracking_url;
        ord.tracking_number = tracking_num;
        ord.payment_method = pay_method;
        ord.payment_status = pay_status;
        orders.push_back(ord);
    }

    return orders;
}

std::optional<Order> OrderModel::getPendingOrderByUserId(int &user_id)
{
    // Get database connection
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0)
    {
        std::cerr << "Error: No active database connection: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    // Prepare the SQL statement
    const char *query =
        "SELECT id, user_id, shipping_address_id, billing_address_id,"
        "       order_date, status, total, shipment_date,"
        "       delivery_date, carrier_id, tracking_url,"
        "       tracking_number, payment_method, payment_status"
        "  FROM orders WHERE user_id = ? AND status = 'pending' LIMIT 1";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Statement initialization failed: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }
    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cerr << "Statement preparation failed: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }
    // Bind the parameters
    MYSQL_BIND param{};
    param.buffer_type = MYSQL_TYPE_LONG;
    param.buffer = &user_id;
    param.buffer_length = sizeof(user_id);
    param.is_null = nullptr;
    param.length = nullptr;
    if (mysql_stmt_bind_param(stmt, &param) != 0)
    {
        std::cerr << "Parameter binding failed: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }
    // Execute the statement
    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Statement execution failed: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }
    // Bind the results
    MYSQL_BIND result_bind[14];
    memset(result_bind, 0, sizeof(result_bind));
    int id, user_id_buffer, shipping_address_id, billing_address_id, carrier_id;
    double total;
    char order_date[64], status[32];
    char ship_date[64], delivery_date[64];
    char tracking_url[128], tracking_num[64];
    char pay_method[64], pay_status[32];
    bool is_null[14];
    unsigned long length[14];

    // Columna 0 → id
    result_bind[0].buffer_type = MYSQL_TYPE_LONG;
    result_bind[0].buffer = &id;
    result_bind[0].buffer_length = sizeof(id);
    result_bind[0].is_null = &is_null[0];
    result_bind[0].length = &length[0];

    // Columna 1 → user_id
    result_bind[1].buffer_type = MYSQL_TYPE_LONG;
    result_bind[1].buffer = &user_id_buffer;
    result_bind[1].buffer_length = sizeof(user_id_buffer);
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
    result_bind[6].buffer_type = MYSQL_TYPE_DOUBLE;
    result_bind[6].buffer = &total;
    result_bind[6].buffer_length = sizeof(total);
    result_bind[6].is_null = &is_null[6];
    result_bind[6].length = &length[6];

    // Columna 6 → ship_date
    result_bind[7].buffer_type = MYSQL_TYPE_STRING;
    result_bind[7].buffer = ship_date;
    result_bind[7].buffer_length = sizeof(ship_date);
    result_bind[7].is_null = &is_null[7];
    result_bind[7].length = &length[7];

    // Columna 7 → delivery_date
    result_bind[8].buffer_type = MYSQL_TYPE_STRING;
    result_bind[8].buffer = delivery_date;
    result_bind[8].buffer_length = sizeof(delivery_date);
    result_bind[8].is_null = &is_null[8];
    result_bind[8].length = &length[8];

    // Columna 8 → carrier
    result_bind[9].buffer_type = MYSQL_TYPE_LONG;
    result_bind[9].buffer = &carrier_id;
    result_bind[9].buffer_length = sizeof(carrier_id);
    result_bind[9].is_null = &is_null[9];
    result_bind[9].length = &length[9];

    // Columna 9 → tracking_url
    result_bind[10].buffer_type = MYSQL_TYPE_STRING;
    result_bind[10].buffer = tracking_url;
    result_bind[10].buffer_length = sizeof(tracking_url);
    result_bind[10].is_null = &is_null[10];
    result_bind[10].length = &length[10];

    // Columna 10 → tracking_num
    result_bind[11].buffer_type = MYSQL_TYPE_STRING;
    result_bind[11].buffer = tracking_num;
    result_bind[11].buffer_length = sizeof(tracking_num);
    result_bind[11].is_null = &is_null[11];
    result_bind[11].length = &length[11];

    // Columna 11 → pay_method
    result_bind[12].buffer_type = MYSQL_TYPE_STRING;
    result_bind[12].buffer = pay_method;
    result_bind[12].buffer_length = sizeof(pay_method);
    result_bind[12].is_null = &is_null[12];
    result_bind[12].length = &length[12];

    // Columna 13 → pay_status
    result_bind[13].buffer_type = MYSQL_TYPE_STRING;
    result_bind[13].buffer = pay_status;
    result_bind[13].buffer_length = sizeof(pay_status);
    result_bind[13].is_null = &is_null[13];
    result_bind[13].length = &length[13];

    // Bind the result
    if (mysql_stmt_bind_result(stmt, result_bind) != 0)
    {
        std::cerr << "Bind result failed: " << mysql_stmt_error(stmt) << "\n";
        return std::nullopt;
    }
    // Fetch the result
    Order order;
    int fetch_result = mysql_stmt_fetch(stmt);
    if (fetch_result == 0)
    {
        // Hay datos: llenar el objeto `Order` y devolverlo
        Order order;
        order.id = id;
        order.user_id = user_id;
        order.shipping_address_id = shipping_address_id;
        order.billing_address_id = billing_address_id;
        order.order_date = order_date;
        order.status = status;
        order.total = total;
        order.shipment_date = ship_date;
        order.delivery_date = delivery_date;
        order.carrier_id = carrier_id;
        order.tracking_url = tracking_url;
        order.tracking_number = tracking_num;
        order.payment_method = pay_method;
        order.payment_status = pay_status;
        return order;
    }
    else if (fetch_result == MYSQL_NO_DATA)
    {
        // No se encontró ninguna orden pendiente para ese user_id
        std::cout << "No pending order found for user_id: " << user_id << std::endl;
        return std::nullopt;
    }
    else
    {
        // Error real
        std::cerr << "Fetch failed OrderModel::getPendingOrderByUserId: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }
}

std::pair<std::optional<Order>, Errors> OrderModel::updateOrder(
    const int &user_id,
    const int &order_id,
    const int &shipping_address_id,
    const int &billing_address_id,
    const std::vector<OrderItem> &products,
    const std::string &shipment_date,
    const std::string &delivery_date,
    const int &carrier_id,
    const std::string &tracking_url,
    const std::string &tracking_number,
    const std::string &payment_method,
    const std::string &payment_status)
{
    // Get database connection
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0)
    {
        std::cerr << "Error: No active database connection: " << mysql_error(conn) << std::endl;
        return {std::nullopt, Errors::DatabaseConnectionFailed};
    }

    // Start transaction to ensure atomicity
    if (mysql_query(conn, "START TRANSACTION") != 0)
    {
        std::cerr << "Error starting transaction checking order exists: " << mysql_error(conn) << std::endl;
        return {std::nullopt, Errors::TransactionStartFailed};
    }

    MYSQL_STMT *checkStmt = nullptr;
    MYSQL_RES *checkResult = nullptr;

    try
    {
        // Check if the order exists and belongs to the user
        const char *checkQuery = "SELECT user_id FROM orders WHERE id = ?";
        checkStmt = mysql_stmt_init(conn);
        if (!checkStmt)
        {
            std::cerr << "Statement initialization failed (check query): " << mysql_error(conn) << std::endl;
            throw std::runtime_error("Statement initialization failed (check query in updateOrder)");
        }
        auto checkStmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(checkStmt, mysql_stmt_close);
        if (mysql_stmt_prepare(checkStmt, checkQuery, strlen(checkQuery)) != 0)
        {
            std::cerr << "Statement preparation failed (check query): " << mysql_stmt_error(checkStmt) << std::endl;
            throw std::runtime_error("Statement preparation failed (check query in updateOrder)");
        }
        // Bind the order ID parameter for the check query
        MYSQL_BIND checkParam[1];
        int order_id_param = order_id;
        bool is_null_checkParam[1];
        unsigned long lenght_checkParam[1];

        memset(checkParam, 0, sizeof(checkParam));
        checkParam[0].buffer_type = MYSQL_TYPE_LONG;
        checkParam[0].buffer = (void *)&order_id_param;
        checkParam[0].buffer_length = sizeof(order_id_param);
        checkParam[0].is_null = &is_null_checkParam[0];
        checkParam[0].length = &lenght_checkParam[0];

        if (mysql_stmt_bind_param(checkStmt, checkParam) != 0)
        {
            std::cerr << "Parameter binding failed (check query): " << mysql_stmt_error(checkStmt) << std::endl;
            throw std::runtime_error("Parameter binding failed (check query in updateOrder)");
        }

        // Execute the check statement
        if (mysql_stmt_execute(checkStmt) != 0)
        {
            std::cerr << "Statement execution failed (check query in updateOrder): " << mysql_stmt_error(checkStmt) << std::endl;
            throw std::runtime_error("Statement execution failed (check query in updateOrder)");
        }

        // Bind the result for the check query
        MYSQL_BIND checkResultBind[1];
        memset(checkResultBind, 0, sizeof(checkResultBind));
        int user_id_result;
        bool is_null_result[1];
        unsigned long lenght_result[1];

        checkResultBind[0].buffer_type = MYSQL_TYPE_LONG;
        checkResultBind[0].buffer = &user_id_result;
        checkResultBind[0].buffer_length = sizeof(user_id_result);
        checkResultBind[0].is_null = &is_null_result[0];
        checkResultBind[0].length = &lenght_result[0];

        // Bind the result for the check query
        if (mysql_stmt_bind_result(checkStmt, checkResultBind) != 0)
        {
            std::cerr << "Bind result failed (check query in updateOrder): " << mysql_stmt_error(checkStmt) << "\n";
            throw std::runtime_error("Bind result failed (check query in updateOrder)");
        }
        // Fetch the result of the check query
        int fetch_result = mysql_stmt_fetch(checkStmt);
        if (fetch_result == MYSQL_NO_DATA)
        {
            std::cerr << "No data found for order_id: " << order_id << "\n";
            throw std::runtime_error("No data found for order_id");
        }
        else if (fetch_result != 0)
        {
            std::cerr << "Fetch failed (check query in update Order): " << mysql_stmt_error(checkStmt) << "\n";
            throw std::runtime_error("Fetch failed (check query in updateOrder)");
        }
        // Verify if the fetched user ID matches the provided user ID
        if (user_id_result != user_id)
        {
            std::cerr << "Error: User ID " << user_id << " does not match the order's user ID in (updateOrder) " << user_id_result << std::endl;
            throw std::runtime_error("User ID does not match");
        }

        mysql_stmt_free_result(checkStmt);

        // commit
        if (mysql_query(conn, "COMMIT") != 0)
        {
            std::cerr << "Error committing transaction in checking order exists: " << mysql_error(conn) << std::endl;
            throw std::runtime_error("Error committing in checking order exits");
        }

        if (mysql_query(conn, "START TRANSACTION") != 0)
        {
            std::cerr << "Error starting transaction update table: " << mysql_error(conn) << std::endl;
            return {std::nullopt, Errors::TransactionStartFailed};
        }

        // Prepare the update statement
        const char *query = "UPDATE orders "
                            "SET shipping_address_id = ?, billing_address_id = ?, "
                            "status = ?, total = ?, shipment_date = ?, delivery_date = ?, "
                            "carrier_id = ?, tracking_url = ?, tracking_number = ?, "
                            "payment_method = ?, payment_status = ? "
                            "WHERE id = ?";
        MYSQL_STMT *stmt = mysql_stmt_init(conn);
        if (!stmt)
        {
            std::cerr << "Statement initialization failed (update query): " << mysql_error(conn) << std::endl;

            throw std::runtime_error("Statement initialization failed (update query)");
        }

        auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);

        if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
        {
            std::cerr << "Statement preparation failed (update query): " << mysql_stmt_error(stmt) << std::endl;
            throw std::runtime_error("Statement preparation failed (update query)");
        }

        double total = orderItemModel.calculateOrderTotal(products);

        // Bind the parameters for the update query
        constexpr size_t NUM_PARAMS = 12;
        MYSQL_BIND param[NUM_PARAMS];
        memset(param, 0, sizeof(param));
        std::string status = "pending";

        // Column 0 → shipping_address_id
        param[0].buffer_type = MYSQL_TYPE_LONG;
        param[0].buffer = (void *)&shipping_address_id;
        param[0].buffer_length = sizeof(shipping_address_id);

        // Column 1 → billing_address_id
        param[1].buffer_type = MYSQL_TYPE_LONG;
        param[1].buffer = (void *)&billing_address_id;
        param[1].buffer_length = sizeof(billing_address_id);

        // Column 2 → status
        param[2].buffer_type = MYSQL_TYPE_STRING;
        param[2].buffer = (void *)status.c_str();
        param[2].buffer_length = status.length();

        // Column 3 → total
        double total_val = total;
        param[3].buffer_type = MYSQL_TYPE_DOUBLE;
        param[3].buffer = (void *)&total_val;
        param[3].buffer_length = sizeof(total_val);

        // Column 4 → shipment_date
        param[4].buffer_type = MYSQL_TYPE_STRING;
        param[4].buffer = (void *)shipment_date.c_str();
        param[4].buffer_length = shipment_date.length();

        // Column 5 → delivery_date
        param[5].buffer_type = MYSQL_TYPE_STRING;
        param[5].buffer = (void *)delivery_date.c_str();
        param[5].buffer_length = delivery_date.length();

        // Column 6 → carrier
        param[6].buffer_type = MYSQL_TYPE_LONG;
        param[6].buffer = (void *)&carrier_id;
        param[6].buffer_length = sizeof(carrier_id);

        // Column 7 → tracking_url
        param[7].buffer_type = MYSQL_TYPE_STRING;
        param[7].buffer = (void *)tracking_url.c_str();
        param[7].buffer_length = tracking_url.length();

        // Column 8 → tracking_number
        param[8].buffer_type = MYSQL_TYPE_STRING;
        param[8].buffer = (void *)tracking_number.c_str();
        param[8].buffer_length = tracking_number.length();

        // Column 9 → payment_method
        param[9].buffer_type = MYSQL_TYPE_STRING;
        param[9].buffer = (void *)payment_method.c_str();
        param[9].buffer_length = payment_method.length();

        // Column 10 → payment_status
        param[10].buffer_type = MYSQL_TYPE_STRING;
        param[10].buffer = (void *)payment_status.c_str();
        param[10].buffer_length = payment_status.length();

        // Column 11 → order_id (for the WHERE clause)
        param[11].buffer_type = MYSQL_TYPE_LONG;
        param[11].buffer = (void *)&order_id;
        param[11].buffer_length = sizeof(order_id);

        // Bind the parameters for the update query
        if (mysql_stmt_bind_param(stmt, param) != 0)
        {
            std::cerr << "Parameter binding failed (update query): " << mysql_stmt_error(stmt) << std::endl;

            throw std::runtime_error("Parameter binding failed (update query)");
        }

        // Execute the update statement
        if (mysql_stmt_execute(stmt) != 0)
        {
            std::cerr << "Statement execution failed (update query): " << mysql_stmt_error(stmt) << std::endl;
            throw std::runtime_error("Statement execution failed (update query)");
        }

        // Check if any rows were affected by the update
        if (mysql_stmt_affected_rows(stmt) == 0)
        {
            std::cerr << "No rows updated for order_id: " << order_id << std::endl;
            mysql_query(conn, "ROLLBACK");
            Order order;
            order.id = order_id;
            return {order, Errors::NoRowsAffected};
        }

        // Commit the transaction
        if (mysql_query(conn, "COMMIT") != 0)
        {
            std::cerr << "Commit failed: " << mysql_error(conn) << std::endl;
            mysql_query(conn, "ROLLBACK");
            throw std::runtime_error("commit failed in update");
        }

        // Fetch the updated order
        Order order;
        order.id = order_id;
        order.user_id = user_id;
        order.shipping_address_id = shipping_address_id;
        order.billing_address_id = billing_address_id;
        order.order_date = ""; // Consider fetching this from the DB if needed
        order.status = status;
        order.total = total;
        order.shipment_date = shipment_date;
        order.delivery_date = delivery_date;
        order.carrier_id = carrier_id;
        order.tracking_url = tracking_url;
        order.tracking_number = tracking_number;
        order.payment_method = payment_method;
        order.payment_status = payment_status;
        return {order, Errors::NoError};
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Error during updateOrder catch: " << e.what() << std::endl;
        if (conn)
        {
            mysql_query(conn, "ROLLBACK"); // Rollback on error
        }
        return {std::nullopt, Errors::CatchError};
    }
    if (checkStmt)
    {
        mysql_stmt_close(checkStmt);
    }
}

std::optional<Order> OrderModel::getOrderById(int &order_id, int &user_id)

// Get database connection
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0)
    {
        std::cerr << "Error: No active database connection: " << mysql_error(conn) << std::endl;
        throw std::runtime_error("No active database connection");
    }

    try
    {

        // Start transaction to ensure atomicity
        if (mysql_query(conn, "START TRANSACTION") != 0)
        {
            std::cerr << "Error starting transaction order exists: " << mysql_error(conn) << std::endl;
            throw std::runtime_error("Error starting transaction getOrderById");
        }
        // Check if the order exists
        const char *checkQuery = "SELECT id FROM orders WHERE id = ?";
        MYSQL_STMT *checkStmt = mysql_stmt_init(conn);
        if (!checkStmt)
        {
            std::cerr << "Statement initialization failed in order exists:: " << mysql_error(conn) << std::endl;
            throw std::runtime_error("Statement initialization failed in order exists:");
        }
        auto checkStmtGuard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(checkStmt, mysql_stmt_close);
        if (mysql_stmt_prepare(checkStmt, checkQuery, strlen(checkQuery)) != 0)
        {
            std::cerr << "Statement preparation failed in order exists:: " << mysql_stmt_error(checkStmt) << std::endl;
            throw std::runtime_error("Statement preparation failed in order exists:");
        }
        MYSQL_BIND checkParam{};
        checkParam.buffer_type = MYSQL_TYPE_LONG;
        checkParam.buffer = &order_id;
        checkParam.buffer_length = sizeof(order_id);
        if (mysql_stmt_bind_param(checkStmt, &checkParam) != 0)
        {
            std::cerr << "Parameter binding failed in order exists: " << mysql_stmt_error(checkStmt) << std::endl;
            throw std::runtime_error("Parameter binding failed in order exists:");
        }
        if (mysql_stmt_execute(checkStmt) != 0)
        {
            std::cerr << "Statement execution failed in order exists: " << mysql_stmt_error(checkStmt) << std::endl;
            throw std::runtime_error("Statement execution failed in order exists:");
        }

        // IMPORTANTE: almacenar el resultado (aunque no lo uses)
        if (mysql_stmt_store_result(checkStmt) != 0)
        {
            std::cerr << "Error storing result in order exists: " << mysql_stmt_error(checkStmt) << std::endl;
            throw std::runtime_error("Error storing result in order exists");
        }

        // Intentar fetch para ver si hay alguna fila
        if (mysql_stmt_fetch(checkStmt) == MYSQL_NO_DATA)
        {
            std::cerr << "Order not found in order exists: " << order_id << std::endl;
            throw std::runtime_error("Order not found in order exists");
        }

        // Commit the transaction
        if (mysql_query(conn, "COMMIT") != 0)
        {
            std::cerr << "Commit failed in order exists: " << mysql_error(conn) << std::endl;
            throw std::runtime_error("Commit failed in order exists");
        }
        // Start a new transaction for fetching the order
        if (mysql_query(conn, "START TRANSACTION") != 0)
        {
            std::cerr << "Error starting transaction CheckUser: " << mysql_error(conn) << std::endl;
            throw std::runtime_error("Error starting transaction CheckUser");
        }

        // Check if the order exists for this user
        const char *checkUserQuery = "SELECT user_id FROM orders WHERE id = ? and user_id = ?";
        MYSQL_STMT *checkUserStmt = mysql_stmt_init(conn);
        if (!checkUserStmt)
        {
            std::cerr << "Statement initialization failed in check user: " << mysql_error(conn) << std::endl;
            throw std::runtime_error("Statement initialization failed in check user");
        }
        auto checkUserStmtGuard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(checkUserStmt, mysql_stmt_close);
        if (mysql_stmt_prepare(checkUserStmt, checkUserQuery, strlen(checkUserQuery)) != 0)
        {
            std::cerr << "Statement preparation failed in check user: " << mysql_stmt_error(checkUserStmt) << std::endl;
            throw std::runtime_error("Statement preparation failed in check user");
        }
        // Bind the parameters for the check user query
        MYSQL_BIND checkUserParam[2];
        memset(checkUserParam, 0, sizeof(checkUserParam));
        checkUserParam[0].buffer_type = MYSQL_TYPE_LONG;
        checkUserParam[0].buffer = &order_id;
        checkUserParam[0].buffer_length = sizeof(order_id);
        checkUserParam[1].buffer_type = MYSQL_TYPE_LONG;
        checkUserParam[1].buffer = &user_id;
        checkUserParam[1].buffer_length = sizeof(user_id);

        // Bind the parameters for the check user query
        if (mysql_stmt_bind_param(checkUserStmt, checkUserParam) != 0)
        {
            std::cerr << "Parameter binding failed in check user: " << mysql_stmt_error(checkUserStmt) << std::endl;
            throw std::runtime_error("Parameter binding failed in check user");
        }

        int fetchedUserId;

        // Definir el array de bind para el resultado
        MYSQL_BIND checkUserResult[1];
        memset(checkUserResult, 0, sizeof(checkUserResult));

        // Configuramos el tipo de dato y la variable donde se almacenará el resultado
        checkUserResult[0].buffer_type = MYSQL_TYPE_LONG;
        checkUserResult[0].buffer = &fetchedUserId; // Es aquí donde almacenaremos el user_id
        checkUserResult[0].buffer_length = sizeof(fetchedUserId);

        // Bind result to the statement
        if (mysql_stmt_bind_result(checkUserStmt, checkUserResult) != 0)
        {
            std::cerr << "Binding result failed: " << mysql_stmt_error(checkUserStmt) << std::endl;
            throw std::runtime_error("Binding result failed in check user");
        }

        if (mysql_stmt_execute(checkUserStmt) != 0)
        {
            std::cerr << "Statement execution failed in check user: " << mysql_stmt_error(checkUserStmt) << std::endl;
            throw std::runtime_error("Statement execution failed in check user");
        }
        int fetchResult = mysql_stmt_fetch(checkUserStmt);
        if (fetchResult == MYSQL_NO_DATA)
        {
            throw std::runtime_error("Order not found for this user");
        }
        else if (fetchResult != 0) // Si hubo un error diferente
        {
            std::cerr << "Fetch error: " << mysql_stmt_error(checkUserStmt) << std::endl;
            throw std::runtime_error("Fetch error in check user");
        }

        if (fetchedUserId != user_id) // Si los user_id no coinciden
        {
            throw std::runtime_error("User ID does not match the order");
        }
        mysql_stmt_free_result(checkUserStmt);

        if (mysql_query(conn, "COMMIT") != 0)
        {
            std::cerr << "Commit failed: " << mysql_error(conn) << std::endl;
            throw std::runtime_error("Commit failed in check user");
        }

        // Start a new transaction for fetching the order
        if (mysql_query(conn, "START TRANSACTION") != 0)
        {
            std::cerr << "Error starting transaction getOrderById: " << mysql_error(conn) << std::endl;
            throw std::runtime_error("Error starting transaction getOrderById");
        }

        // Prepare the SQL statement
        const char *query =
            "SELECT id, user_id, shipping_address_id, billing_address_id,"
            "       order_date, status, total, shipment_date,"
            "       delivery_date, carrier_id, tracking_url,"
            "       tracking_number, payment_method, payment_status, paypal_order_id, observations"
            "  FROM orders WHERE id = ?";
        MYSQL_STMT *stmt = mysql_stmt_init(conn);
        if (!stmt)
        {
            std::cerr << "Statement initialization failed: " << mysql_error(conn) << std::endl;
            throw std::runtime_error("Statement initialization failed");
        }
        auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);
        if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
        {
            std::cerr << "Statement preparation failed: " << mysql_stmt_error(stmt) << std::endl;
            throw std::runtime_error("Statement preparation failed");
        }

        // Bind the parameters
        MYSQL_BIND param{};
        param.buffer_type = MYSQL_TYPE_LONG;
        param.buffer = &order_id;
        param.buffer_length = sizeof(order_id);

        if (mysql_stmt_bind_param(stmt, &param) != 0)
        {
            std::cerr << "Parameter binding failed: " << mysql_stmt_error(stmt) << std::endl;
            throw std::runtime_error("Parameter binding failed");
        }

        // Execute the statement
        if (mysql_stmt_execute(stmt) != 0)
        {
            std::cerr << "Statement execution failed: " << mysql_stmt_error(stmt) << std::endl;
            throw std::runtime_error("Statement execution failed");
        }

        // Bind the results
        MYSQL_BIND result_bind[16];
        memset(result_bind, 0, sizeof(result_bind));
        int id, user_id_query, shipping_address_id, billing_address_id, carrier_id;
        double total;
        char order_date[64], status[32];
        char ship_date[64], delivery_date[64];
        char tracking_url[256], tracking_number[64];
        char payment_method[64], payment_status[64], paypal_order_id[64], observations[256];

        bool is_null_paypal_order_id;

        result_bind[0].buffer_type = MYSQL_TYPE_LONG;
        result_bind[0].buffer = &id;
        result_bind[0].buffer_length = sizeof(id);

        result_bind[1].buffer_type = MYSQL_TYPE_LONG;
        result_bind[1].buffer = &user_id_query;
        result_bind[1].buffer_length = sizeof(user_id_query);

        result_bind[2].buffer_type = MYSQL_TYPE_LONG;
        result_bind[2].buffer = &shipping_address_id;
        result_bind[2].buffer_length = sizeof(shipping_address_id);

        result_bind[3].buffer_type = MYSQL_TYPE_LONG;
        result_bind[3].buffer = &billing_address_id;
        result_bind[3].buffer_length = sizeof(billing_address_id);

        result_bind[4].buffer_type = MYSQL_TYPE_STRING;
        result_bind[4].buffer = order_date;
        result_bind[4].buffer_length = sizeof(order_date);

        result_bind[5].buffer_type = MYSQL_TYPE_STRING;
        result_bind[5].buffer = status;
        result_bind[5].buffer_length = sizeof(status);

        result_bind[6].buffer_type = MYSQL_TYPE_DOUBLE;
        result_bind[6].buffer = &total;
        result_bind[6].buffer_length = sizeof(total);

        result_bind[7].buffer_type = MYSQL_TYPE_STRING;
        result_bind[7].buffer = ship_date;
        result_bind[7].buffer_length = sizeof(ship_date);

        result_bind[8].buffer_type = MYSQL_TYPE_STRING;
        result_bind[8].buffer = delivery_date;
        result_bind[8].buffer_length = sizeof(delivery_date);

        result_bind[9].buffer_type = MYSQL_TYPE_LONG;
        result_bind[9].buffer = &carrier_id;
        result_bind[9].buffer_length = sizeof(carrier_id);

        result_bind[10].buffer_type = MYSQL_TYPE_STRING;
        result_bind[10].buffer = tracking_url;
        result_bind[10].buffer_length = sizeof(tracking_url);

        result_bind[11].buffer_type = MYSQL_TYPE_STRING;
        result_bind[11].buffer = tracking_number;
        result_bind[11].buffer_length = sizeof(tracking_number);

        result_bind[12].buffer_type = MYSQL_TYPE_STRING;
        result_bind[12].buffer = payment_method;
        result_bind[12].buffer_length = sizeof(payment_method);

        result_bind[13].buffer_type = MYSQL_TYPE_STRING;
        result_bind[13].buffer = payment_status;
        result_bind[13].buffer_length = sizeof(payment_status);

        result_bind[14].buffer_type = MYSQL_TYPE_STRING;
        result_bind[14].buffer = paypal_order_id;
        result_bind[14].buffer_length = sizeof(paypal_order_id);
        result_bind[14].is_null = &is_null_paypal_order_id;

        result_bind[15].buffer_type = MYSQL_TYPE_STRING;
        result_bind[15].buffer = observations;
        result_bind[15].buffer_length = sizeof(observations);

        // Bind the result
        if (mysql_stmt_bind_result(stmt, result_bind) != 0)
        {
            std::cerr << "Result binding failed: " << mysql_stmt_error(stmt) << std::endl;
            throw std::runtime_error("Result binding failed");
        }
        if (mysql_stmt_fetch(stmt) != 0)
        {
            std::cerr << "Fetch failed OrderModel -getOrderByID-: " << mysql_stmt_error(stmt) << std::endl;
            throw std::runtime_error("Fetch failed OrderModel -getOrderByID-");
        }

        Order order;
        order.id = id;
        order.user_id = user_id_query;
        order.shipping_address_id = shipping_address_id;
        order.billing_address_id = billing_address_id;
        order.order_date = order_date;
        order.status = status;
        order.total = total;
        order.shipment_date = ship_date;
        order.delivery_date = delivery_date;
        order.carrier_id = carrier_id;
        order.tracking_url = tracking_url;
        order.tracking_number = tracking_number;
        order.payment_method = payment_method;
        order.payment_status = payment_status;
        order.paypal_order_id = paypal_order_id;
        order.observations = observations;
        // Commit the transaction

        if (is_null_paypal_order_id)
        {
            order.paypal_order_id = "";
        }

        mysql_stmt_free_result(stmt);
        if (mysql_query(conn, "COMMIT") != 0)
        {
            std::cerr << "Commit failed: " << mysql_error(conn) << std::endl;
            throw std::runtime_error("Commit failed");
        }
        return order;
    }
    catch (const std::exception &e)
    {
        mysql_query(conn, "ROLLBACK");
        std::cerr << "Error in getOrderById: " << e.what() << std::endl;
        return std::nullopt;
    }
    // Get database connection
}

std::pair<std::optional<Order>, Errors> OrderModel::updateOrderPaypalId(const int &user_id, const int &order_id, const std::string &payment_id)
{
    // Get database connection
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();

    if (!conn)
    {
        std::cerr << "Error: No active database connection: " << mysql_error(conn) << std::endl;
        return std::make_pair(std::nullopt, Errors::DatabaseConnectionFailed);
    }

    try
    {

        // Start transaction
        if (mysql_query(conn, "START TRANSACTION") != 0)
        {
            std::cerr << "Start transaction failed: " << mysql_error(conn) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return std::make_pair(std::nullopt, Errors::TransactionStartFailed);
        }
        // Prepare the SQL statement
        const char *sql = "UPDATE orders SET paypal_order_id = ? WHERE user_id = ? AND id = ?";
        MYSQL_STMT *stmt = mysql_stmt_init(conn);
        if (!stmt)
        {
            std::cerr << "Statement initialization failed: " << mysql_error(conn) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return std::make_pair(std::nullopt, Errors::StatementInitFailed);
        }
        auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);
        if (mysql_stmt_prepare(stmt, sql, strlen(sql)) != 0)
        {
            std::cerr << "Statement preparation failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return std::make_pair(std::nullopt, Errors::StatementPrepareFailed);
        }
        // Bind the parameters
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        unsigned long payment_id_len = payment_id.size();

        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (void *)payment_id.c_str();
        bind[0].buffer_length = payment_id_len;
        bind[0].is_null = 0;
        bind[0].length = &payment_id_len;

        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = (void *)&user_id;
        bind[1].is_null = 0;
        bind[1].length = 0;

        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = (void *)&order_id;
        bind[2].is_null = 0;
        bind[2].length = 0;

        if (mysql_stmt_bind_param(stmt, bind) != 0)
        {
            std::cerr << "Parameter binding failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return std::make_pair(std::nullopt, Errors::BindParamFailed);
        }
        // Execute the statement
        if (mysql_stmt_execute(stmt) != 0)
        {
            std::cerr << "Statement execution failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return std::make_pair(std::nullopt, Errors::ExecutionFailed);
        }
        // Check if any rows were affected
        if (mysql_stmt_affected_rows(stmt) == 0)
        {
            std::cerr << "No rows updated: " << mysql_stmt_error(stmt) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return std::make_pair(std::nullopt, Errors::NoRowsAffected);
        }
        // Commit the transaction
        if (mysql_query(conn, "COMMIT") != 0)
        {
            std::cerr << "Commit failed: " << mysql_error(conn) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return std::make_pair(std::nullopt, Errors::CommitFailed);
        }
        // Fetch the updated order
        Order order;
        order.id = order_id;
        order.user_id = user_id;
        return std::make_pair(order, Errors::NoError);
    }
    catch (const std::exception &e)
    {
        mysql_query(conn, "ROLLBACK");
        std::cerr << "Error in updateOrderPaypalId: " << e.what() << std::endl;
        mysql_query(conn, "ROLLBACK");
        return std::make_pair(std::nullopt, Errors::UnknownError);
    }
}

std::pair<bool, Errors> OrderModel::updateOrderTotal(int order_id, double total)
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0)
    {
        std::cerr << "Error: No active database connection: " << mysql_error(conn) << std::endl;
        return {false, Errors::DatabaseConnectionFailed};
    }

    if (mysql_query(conn, "START TRANSACTION") != 0)
    {
        std::cerr << "Error starting transaction in updateOrderTotal: " << mysql_error(conn) << std::endl;
        return {false, Errors::TransactionStartFailed};
    }

    const char *query = "UPDATE orders SET total = ? WHERE id = ?";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Statement init failed in updateOrderTotal: " << mysql_error(conn) << std::endl;
        return {false, Errors::StatementInitFailed};
    }

    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cerr << "Statement prepare failed in updateOrderTotal: " << mysql_stmt_error(stmt) << std::endl;
        return {false, Errors::StatementPrepareFailed};
    }

    MYSQL_BIND param[2];
    memset(param, 0, sizeof(param));
    bool is_null[2];
    unsigned long length[2];

    param[0].buffer_type = MYSQL_TYPE_DOUBLE;
    param[0].buffer = &total;
    param[0].is_null = &is_null[0];
    param[0].length = &length[0];

    param[1].buffer_type = MYSQL_TYPE_LONG;
    param[1].buffer = &order_id;
    param[1].is_null = &is_null[1];
    param[1].length = &length[1];

    if (mysql_stmt_bind_param(stmt, param) != 0)
    {
        std::cerr << "Parameter binding failed in updateOrderTotal: " << mysql_stmt_error(stmt) << std::endl;
        return {false, Errors::BindParamFailed};
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Statement execution failed in updateOrderTotal: " << mysql_stmt_error(stmt) << std::endl;
        return {false, Errors::ExecutionFailed};
    }

    if (mysql_stmt_affected_rows(stmt) == 0)
    {
        std::cerr << "No rows updated in updateOrderTotal: " << mysql_stmt_error(stmt) << std::endl;
        return {true, Errors::NoRowsAffected};
    }

    if (mysql_query(conn, "COMMIT") != 0)
    {
        std::cerr << "Commit failed in updateOrderTotal: " << mysql_error(conn) << std::endl;
        return {false, Errors::CommitFailed};
    }

    return {true, Errors::NoError};
}

std::pair<bool, Errors> OrderModel::updateCarrierId(int order_id, int carrier_id)
{

    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0)
    {
        std::cerr << "Error: No active database connection: " << mysql_error(conn) << std::endl;
        return {false, Errors::DatabaseConnectionFailed};
    }

    if (mysql_query(conn, "START TRANSACTION") != 0)
    {
        std::cerr << "Error starting transaction in updateOrderTotal: " << mysql_error(conn) << std::endl;
        return {false, Errors::TransactionStartFailed};
    }

    const char *query = "UPDATE orders SET carrier_id = ? WHERE id = ?";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Statement init failed in updateOrderTotal: " << mysql_error(conn) << std::endl;
        return {false, Errors::StatementInitFailed};
    }

    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cerr << "Statement prepare failed in updateOrderTotal: " << mysql_stmt_error(stmt) << std::endl;
        return {false, Errors::StatementPrepareFailed};
    }

    MYSQL_BIND param[2];
    memset(param, 0, sizeof(param));
    bool is_null[2];
    unsigned long length[2];

    param[0].buffer_type = MYSQL_TYPE_LONG;
    param[0].buffer = &carrier_id;
    param[0].is_null = &is_null[0];
    param[0].length = &length[0];

    param[1].buffer_type = MYSQL_TYPE_LONG;
    param[1].buffer = &order_id;
    param[1].is_null = &is_null[1];
    param[1].length = &length[1];

    if (mysql_stmt_bind_param(stmt, param) != 0)
    {
        std::cerr << "Parameter binding failed in updateOrderTotal: " << mysql_stmt_error(stmt) << std::endl;
        return {false, Errors::BindParamFailed};
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Statement execution failed in updateOrderTotal: " << mysql_stmt_error(stmt) << std::endl;
        return {false, Errors::ExecutionFailed};
    }

    if (mysql_stmt_affected_rows(stmt) == 0)
    {
        std::cerr << "No rows updated in updateOrderTotal: " << mysql_stmt_error(stmt) << std::endl;
        return {true, Errors::NoRowsAffected};
    }

    if (mysql_query(conn, "COMMIT") != 0)
    {
        std::cerr << "Commit failed in updateOrderTotal: " << mysql_error(conn) << std::endl;
        return {false, Errors::CommitFailed};
    }

    return {true, Errors::NoError};
}

std::pair<std::optional<Order>, Errors> OrderModel::updateOrderStatus(
    int &user_id,
    int &order_id,
    const std::string &status)
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0)
    {
        std::cerr << "Error: No active database connection: " << mysql_error(conn) << std::endl;
        return {std::nullopt, Errors::DatabaseConnectionFailed};
    }

    try
    {
        // Start transaction
        if (mysql_query(conn, "START TRANSACTION") != 0)
        {
            std::cerr << "Start transaction failed: " << mysql_error(conn) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return {std::nullopt, Errors::TransactionStartFailed};
        }

        // Update order status
        const char *query = "UPDATE orders SET status = ? WHERE id = ? AND user_id = ?";
        MYSQL_STMT *stmt = mysql_stmt_init(conn);
        if (!stmt)
        {
            std::cerr << "Statement init failed: " << mysql_error(conn) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return {std::nullopt, Errors::StatementInitFailed};
        }

        auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);

        if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
        {
            std::cerr << "Statement prepare failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return {std::nullopt, Errors::StatementPrepareFailed};
        }

        MYSQL_BIND param[3];
        memset(param, 0, sizeof(param));
        bool is_null[3];
        unsigned long length[3];

        param[0].buffer_type = MYSQL_TYPE_STRING;
        param[0].buffer = (char *)status.c_str();
        param[0].is_null = &is_null[0];
        param[0].length = &length[0];

        param[1].buffer_type = MYSQL_TYPE_LONG;
        param[1].buffer = &order_id;
        param[1].is_null = &is_null[1];
        param[1].length = &length[1];

        param[2].buffer_type = MYSQL_TYPE_LONG;
        param[2].buffer = &user_id;
        param[2].is_null = &is_null[2];
        param[2].length = &length[2];

        if (mysql_stmt_bind_param(stmt, param) != 0)
        {
            std::cerr << "Parameter binding failed in updateOrderStatus: " << mysql_stmt_error(stmt) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return {std::nullopt, Errors::BindParamFailed};
        }

        if (mysql_stmt_execute(stmt) != 0)
        {
            std::cerr << "Statement execution failed in updateOrderStatus: " << mysql_stmt_error(stmt) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return {std::nullopt, Errors::ExecutionFailed};
        }

        if (mysql_stmt_affected_rows(stmt) == 0)
        {
            std::cerr << "No rows updated in updateOrderStatus: " << mysql_stmt_error(stmt) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return {std::nullopt, Errors::NoRowsAffected};
        }

        if (mysql_query(conn, "COMMIT") != 0)
        {
            std::cerr << "Commit failed in updateOrderStatus: " << mysql_error(conn) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return {std::nullopt, Errors::CommitFailed};
        }

        return {std::nullopt, Errors::NoError};
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception caught in updateOrderStatus: " << e.what() << std::endl;
        mysql_query(conn, "ROLLBACK");
        return {std::nullopt, Errors::UnknownError};
    }
}