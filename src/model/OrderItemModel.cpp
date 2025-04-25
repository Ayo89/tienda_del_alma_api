#include "model/OrderItemModel.h"
#include <map>

std::optional<int> OrderItemModel::createOrderItem(const std::vector<OrderItem> &products, int order_id)
{
    // Get a database connection instance
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Error: No active database connection: " << mysql_error(conn) << std::endl;
        return std::nullopt; // Return null if no connection
    }

    // Start a transaction
    if (mysql_query(conn, "START TRANSACTION") != 0)
    {
        std::cerr << "Error starting transaction: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    // SQL query for inserting new items into order_items table
    const char *sql =
        "INSERT INTO order_items (order_id, product_id, quantity, price) VALUES (?, ?, ?, ?)";

    // Initialize the statement
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Statement initialization failed: " << mysql_error(conn) << std::endl;
        mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
        return std::nullopt;
    }

    // Using a smart pointer to automatically manage the resource (stmt)
    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);

    // Prepare the statement for execution
    if (mysql_stmt_prepare(stmt, sql, strlen(sql)) != 0)
    {
        std::cerr << "Statement preparation failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
        return std::nullopt;
    }

    // Iterate through the products and insert them one by one
    for (const auto &item : products)
    {
        int product_id = item.product_id;
        int quantity = item.quantity;
        double price = item.price;

        // Bind parameters to the statement
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

        // Bind parameters to the statement
        if (mysql_stmt_bind_param(stmt, param_bind) != 0)
        {
            std::cerr << "Parameter binding failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
            return std::nullopt;
        }

        // Execute the statement
        if (mysql_stmt_execute(stmt) != 0)
        {
            std::cerr << "Statement execution failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
            return std::nullopt;
        }
    }

    // Commit the transaction
    if (mysql_query(conn, "COMMIT") != 0)
    {
        std::cerr << "Error committing transaction: " << mysql_error(conn) << std::endl;
        mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
        return std::nullopt;
    }

    // Return the last inserted ID
    return mysql_insert_id(conn);
}

std::optional<int> OrderItemModel::updateOrderItems(const std::vector<OrderItem> &products, int order_id)
{
    // Get a database connection instance
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Error: No active database connection: " << mysql_error(conn) << std::endl;
        return std::nullopt; // Return null if no connection
    }

    // Start a transaction
    if (mysql_query(conn, "START TRANSACTION") != 0)
    {
        std::cerr << "Error starting transaction: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    // SQL query for updating items in order_items table
    const char *sql =
        "UPDATE order_items SET quantity = ?, price = ? WHERE order_id = ? AND product_id = ?";

    // Initialize the statement
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Statement initialization failed: " << mysql_error(conn) << std::endl;
        mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
        return std::nullopt;
    }

    // Using a smart pointer to automatically manage the resource (stmt)
    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);

    // Prepare the statement for execution
    if (mysql_stmt_prepare(stmt, sql, strlen(sql)) != 0)
    {
        std::cerr << "Statement preparation failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
        return std::nullopt;
    }

    // Iterate through the products and update them
    for (const auto &item : products)
    {
        int product_id = item.product_id;
        int quantity = item.quantity;
        double price = item.price;

        // Bind parameters to the statement
        MYSQL_BIND param_bind[4];
        memset(param_bind, 0, sizeof(param_bind));

        param_bind[0].buffer_type = MYSQL_TYPE_LONG;
        param_bind[0].buffer = &quantity;

        param_bind[1].buffer_type = MYSQL_TYPE_DOUBLE;
        param_bind[1].buffer = &price;

        param_bind[2].buffer_type = MYSQL_TYPE_LONG;
        param_bind[2].buffer = &order_id;

        param_bind[3].buffer_type = MYSQL_TYPE_LONG;
        param_bind[3].buffer = &product_id;

        // Bind parameters to the statement
        if (mysql_stmt_bind_param(stmt, param_bind) != 0)
        {
            std::cerr << "Parameter binding failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
            return std::nullopt;
        }

        // Execute the statement
        if (mysql_stmt_execute(stmt) != 0)
        {
            std::cerr << "Statement execution failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
            return std::nullopt;
        }
    }

    // Commit the transaction
    if (mysql_query(conn, "COMMIT") != 0)
    {
        std::cerr << "Error committing transaction: " << mysql_error(conn) << std::endl;
        mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
        return std::nullopt;
    }

    // Return the number of affected rows (updates made)
    return mysql_affected_rows(conn);
}

std::optional<int> OrderItemModel::syncOrderItems(const std::vector<OrderItem> &newItems, int order_id)
{
    // Get a database connection instance
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Error: No active database connection" << std::endl;
        return std::nullopt;
    }

    // Start a transaction
    if (mysql_query(conn, "START TRANSACTION") != 0)
    {
        std::cerr << "Error starting transaction: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    // Get existing items for the given order_id
    std::map<int, std::pair<int, double>> existingItems;
    std::string select_query = "SELECT product_id, quantity, price FROM order_items WHERE order_id = ? ";

    MYSQL_STMT *select_stmt = mysql_stmt_init(conn);
    if (!select_stmt)
    {
        std::cerr << "Statement initialization failed: " << mysql_error(conn) << std::endl;
        mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
        return std::nullopt;
    }

    // Using a smart pointer to automatically manage the resource (select_stmt)
    auto select_stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(select_stmt, mysql_stmt_close);

    // Prepare the SELECT statement to get existing items
    if (mysql_stmt_prepare(select_stmt, select_query.c_str(), select_query.size()) != 0)
    {
        std::cerr << "Statement preparation failed in syncOrderItems: " << mysql_stmt_error(select_stmt) << std::endl;
        mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
        return std::nullopt;
    }

    // Bind the order_id to the SELECT statement
    MYSQL_BIND param[1] = {};
    param[0].buffer_type = MYSQL_TYPE_LONG;
    param[0].buffer = &order_id;

    if (mysql_stmt_bind_param(select_stmt, param) != 0)
    {
        std::cerr << "Parameter binding failed: " << mysql_stmt_error(select_stmt) << std::endl;
        mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
        return std::nullopt;
    }

    // Execute the SELECT statement
    if (mysql_stmt_execute(select_stmt) != 0)
    {
        std::cerr << "Statement execution failed: " << mysql_stmt_error(select_stmt) << std::endl;
        mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
        return std::nullopt;
    }

    // Prepare to fetch results from the SELECT statement
    MYSQL_BIND result_bind[3] = {};
    int product_id;
    int quantity;
    double price;

    result_bind[0].buffer_type = MYSQL_TYPE_LONG;
    result_bind[0].buffer = &product_id;
    result_bind[1].buffer_type = MYSQL_TYPE_LONG;
    result_bind[1].buffer = &quantity;
    result_bind[2].buffer_type = MYSQL_TYPE_DOUBLE;
    result_bind[2].buffer = &price;

    if (mysql_stmt_bind_result(select_stmt, result_bind) != 0)
    {
        std::cerr << "Bind result failed: " << mysql_stmt_error(select_stmt) << std::endl;
        mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
        return std::nullopt;
    }

    // Fetch all existing items from the SELECT query and store them in a map
    while (mysql_stmt_fetch(select_stmt) == 0)
    {
        existingItems[product_id] = {quantity, price};
    }
    mysql_stmt_free_result(select_stmt);

    if (mysql_stmt_errno(select_stmt) != 0)
    {
        std::cerr << "Fetch failed: " << mysql_stmt_error(select_stmt) << std::endl;
        mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
        return std::nullopt;
    }

    // Prepare SQL queries for UPDATE, INSERT, and DELETE operations
    const char *update_sql = "UPDATE order_items SET quantity = ?, price = ? WHERE order_id = ? AND product_id = ?";
    const char *insert_sql = "INSERT INTO order_items (order_id, product_id, quantity, price) VALUES (?, ?, ?, ?)";
    const char *delete_sql = "DELETE FROM order_items WHERE order_id = ? AND product_id = ?";

    MYSQL_STMT *update_stmt = mysql_stmt_init(conn);
    MYSQL_STMT *insert_stmt = mysql_stmt_init(conn);
    MYSQL_STMT *delete_stmt = mysql_stmt_init(conn);

    if (!update_stmt || !insert_stmt || !delete_stmt)
    {
        std::cerr << "Statement initialization failed: " << mysql_error(conn) << std::endl;
        mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
        return std::nullopt;
    }

    // Using smart pointers to manage the statement resources
    auto delete_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(delete_stmt, mysql_stmt_close);
    auto update_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(update_stmt, mysql_stmt_close);
    auto insert_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(insert_stmt, mysql_stmt_close);

    // Prepare all the SQL statements
    if (mysql_stmt_prepare(update_stmt, update_sql, strlen(update_sql)) != 0 ||
        mysql_stmt_prepare(insert_stmt, insert_sql, strlen(insert_sql)) != 0 ||
        mysql_stmt_prepare(delete_stmt, delete_sql, strlen(delete_sql)) != 0)
    {
        std::cerr << "Statement preparation failed in syncOrderItems: " << mysql_stmt_error(update_stmt) << std::endl;
        mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
        return std::nullopt;
    }

    // Iterate over the new items to synchronize
    for (const auto &item : newItems)
    {
        int product_id = item.product_id;
        int new_quantity = item.quantity;
        double new_price = item.price;

        auto it = existingItems.find(product_id);

        // Check if the product is already in the existing items
        if (it != existingItems.end())
        {
            int existing_quantity = it->second.first;
            double existing_price = it->second.second;

            // If the quantity or price has changed, perform an UPDATE
            if (new_quantity != existing_quantity || new_price != existing_price)
            {
                // Bind parameters for the UPDATE statement
                MYSQL_BIND update_bind[4] = {};
                update_bind[0].buffer_type = MYSQL_TYPE_LONG;
                update_bind[0].buffer = &new_quantity;
                update_bind[1].buffer_type = MYSQL_TYPE_DOUBLE;
                update_bind[1].buffer = &new_price;
                update_bind[2].buffer_type = MYSQL_TYPE_LONG;
                update_bind[2].buffer = &order_id;
                update_bind[3].buffer_type = MYSQL_TYPE_LONG;
                update_bind[3].buffer = &product_id;

                // Bind parameters to the update statement
                if (mysql_stmt_bind_param(update_stmt, update_bind) != 0)
                {
                    std::cerr << "Update binding failed: " << mysql_stmt_error(update_stmt) << std::endl;
                    mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
                    return std::nullopt;
                }

                // Execute the update statement
                if (mysql_stmt_execute(update_stmt) != 0)
                {
                    std::cerr << "Update failed: " << mysql_stmt_error(update_stmt) << std::endl;
                    mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
                    return std::nullopt;
                }
            }
            existingItems.erase(it); // Remove the item from existingItems after update
        }
        else
        {
            // If the product is not found, we need to insert a new item
            // Bind parameters for the INSERT statement
            MYSQL_BIND insert_bind[4] = {};
            insert_bind[0].buffer_type = MYSQL_TYPE_LONG;
            insert_bind[0].buffer = &order_id;
            insert_bind[1].buffer_type = MYSQL_TYPE_LONG;
            insert_bind[1].buffer = &product_id;
            insert_bind[2].buffer_type = MYSQL_TYPE_LONG;
            insert_bind[2].buffer = &new_quantity;
            insert_bind[3].buffer_type = MYSQL_TYPE_DOUBLE;
            insert_bind[3].buffer = &new_price;

            // Bind parameters to the insert statement
            if (mysql_stmt_bind_param(insert_stmt, insert_bind) != 0)
            {
                std::cerr << "Insert binding failed: " << mysql_stmt_error(insert_stmt) << std::endl;
                mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
                return std::nullopt;
            }

            // Execute the insert statement
            if (mysql_stmt_execute(insert_stmt) != 0)
            {
                std::cerr << "Insert failed: " << mysql_stmt_error(insert_stmt) << std::endl;
                mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
                return std::nullopt;
            }
        }
    }

    // Perform a delete for the remaining products in existingItems (which are no longer in newItems)
    for (const auto &item : existingItems)
    {
        int product_id = item.first;

        // Bind parameters for the DELETE statement
        MYSQL_BIND delete_bind[2] = {};
        delete_bind[0].buffer_type = MYSQL_TYPE_LONG;
        delete_bind[0].buffer = &order_id;
        delete_bind[1].buffer_type = MYSQL_TYPE_LONG;
        delete_bind[1].buffer = &product_id;

        // Bind parameters to the delete statement
        if (mysql_stmt_bind_param(delete_stmt, delete_bind) != 0)
        {
            std::cerr << "Delete binding failed: " << mysql_stmt_error(delete_stmt) << std::endl;
            mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
            return std::nullopt;
        }

        // Execute the delete statement
        if (mysql_stmt_execute(delete_stmt) != 0)
        {
            std::cerr << "Delete failed: " << mysql_stmt_error(delete_stmt) << std::endl;
            mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
            return std::nullopt;
        }
    }

    // Commit the transaction
    if (mysql_query(conn, "COMMIT") != 0)
    {
        std::cerr << "Commit failed: " << mysql_error(conn) << std::endl;
        mysql_query(conn, "ROLLBACK"); // Rollback transaction in case of error
        return std::nullopt;
    }

    return 1; // Indicating successful synchronization
}
