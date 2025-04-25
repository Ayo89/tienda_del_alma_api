#include "model/OrderItemModel.h"

std::optional<int> OrderItemModel::createOrderItem(const std::vector<OrderProduct> &products, int order_id)
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Error: No active database connection: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    if (mysql_query(conn, "START TRANSACTION") != 0)
    {
        std::cerr << "Error starting transaction: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    const char *sql =
        "INSERT INTO order_items (order_id, product_id, quantity, price) VALUES (?, ?, ?, ?)";

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

    for (const auto &item : products)
    {
        int product_id = item.product_id;
        int quantity = item.quantity;
        double price = item.price;

        MYSQL_BIND param_bind[4];
        memset(param_bind, 0, sizeof(param_bind));

        param_bind[0].buffer_type = MYSQL_TYPE_LONG;
        param_bind[0].buffer = &order_id;

        param_bind[1].buffer_type = MYSQL_TYPE_LONG;
        param_bind[1].buffer = &product_id;

        param_bind[2].buffer_type = MYSQL_TYPE_LONG;
        param_bind[2].buffer = &quantity;

        param_bind[3].buffer_type = MYSQL_TYPE_DOUBLE;
        param_bind[3].buffer = &price;

        if (mysql_stmt_bind_param(stmt, param_bind) != 0)
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
    }

    if (mysql_query(conn, "COMMIT") != 0)
    {
        std::cerr << "Error committing transaction: " << mysql_error(conn) << std::endl;
        mysql_query(conn, "ROLLBACK");
        return std::nullopt;
    }

    return mysql_insert_id(conn); // devolvería el último ID insertado
}
