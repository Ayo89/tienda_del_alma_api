#include "model/PaymentAttemptModel.h"

std::pair<std::optional<PaymentAttempt>, Errors> PaymentAttempModel::createPaymentAttempt(int user_id, int order_id, std::string cart_hash, double total, std::string idempotency_key, std::string paypal_order_id, std::string status)
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Failed to get database connection" << std::endl;
        return std::make_pair(std::nullopt, Errors::DatabaseConnectionFailed);
    }

    const char *query = "INSERT INTO payment_attempts (user_id, order_id, cart_hash, total, idempotency_key, paypal_order_id, status) VALUES (?, ?, ?, ?, ?, ?, ?)";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Failed to initialize statement" << std::endl;
        return std::make_pair(std::nullopt, Errors::StatementInitFailed);
    }

    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cerr << "Failed to prepare statement: " << mysql_error(conn) << std::endl;
        return std::make_pair(std::nullopt, Errors::StatementPrepareFailed);
    }

    MYSQL_BIND bind[7];
    memset(bind, 0, sizeof(bind));

    unsigned long len_cart_hash = cart_hash.size();
    unsigned long len_idempotency_key = idempotency_key.size();
    unsigned long len_paypal_order_id = paypal_order_id.size();
    unsigned long len_status = status.size();

    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = &user_id;

    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &order_id;

    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (char *)cart_hash.c_str();
    bind[2].buffer_length = len_cart_hash;

    bind[3].buffer_type = MYSQL_TYPE_DOUBLE;
    bind[3].buffer = &total;

    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].buffer = (char *)idempotency_key.c_str();
    bind[4].buffer_length = len_idempotency_key;

    bind[5].buffer_type = MYSQL_TYPE_STRING;
    bind[5].buffer = (char *)paypal_order_id.c_str();
    bind[5].buffer_length = len_paypal_order_id;

    bind[6].buffer_type = MYSQL_TYPE_STRING;
    bind[6].buffer = (char *)status.c_str();
    bind[6].buffer_length = len_status;

    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        std::cerr << "Failed to bind parameters: " << mysql_error(conn) << std::endl;
        return std::make_pair(std::nullopt, Errors::BindParamFailed);
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Failed to execute statement: " << mysql_error(conn) << std::endl;
        return std::make_pair(std::nullopt, Errors::ExecutionFailed);
    }

    PaymentAttempt paymentAttempt;
    paymentAttempt.id = mysql_stmt_insert_id(stmt);
    paymentAttempt.user_id = user_id;
    paymentAttempt.order_id = order_id;
    paymentAttempt.cart_hash = cart_hash;
    paymentAttempt.total = total;
    paymentAttempt.idempotency_key = idempotency_key;
    paymentAttempt.paypal_order_id = paypal_order_id;
    paymentAttempt.status = status;

    return std::make_pair(paymentAttempt, Errors::NoError);
}
std::pair<std::optional<std::vector<PaymentAttempt>>, Errors> PaymentAttempModel::getPaymentAttemptsByOrderId(int order_id)
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Failed to get database connection" << std::endl;
        return {std::nullopt, Errors::DatabaseConnectionFailed};
    }

    const char *query =
        "SELECT id, user_id, order_id, cart_hash, total, idempotency_key, paypal_order_id, status"
        "  FROM payment_attempts"
        " WHERE order_id = ? AND status = 'PAYER_ACTION_REQUIRED'";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Failed to initialize statement" << std::endl;
        return {std::nullopt, Errors::StatementInitFailed};
    }
    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cerr << "Failed to prepare statement: " << mysql_error(conn) << std::endl;
        return {std::nullopt, Errors::StatementPrepareFailed};
    }

    MYSQL_BIND bind_param[1];
    memset(bind_param, 0, sizeof(bind_param));
    bind_param[0].buffer_type = MYSQL_TYPE_LONG;
    bind_param[0].buffer = &order_id;
    bind_param[0].buffer_length = sizeof(order_id);
    if (mysql_stmt_bind_param(stmt, bind_param) != 0)
    {
        std::cerr << "Failed to bind parameters: " << mysql_error(conn) << std::endl;
        return {std::nullopt, Errors::BindParamFailed};
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Failed to execute statement: " << mysql_error(conn) << std::endl;
        return {std::nullopt, Errors::ExecutionFailed};
    }

    // Allocate result buffers
    MYSQL_BIND result_bind[8];
    memset(result_bind, 0, sizeof(result_bind));

    int id;
    int user_id_buf;
    int order_id_buf;
    double total;
    char cart_hash_buf[256];
    char idemp_key_buf[256];
    char paypal_order_buf[256];
    char status_buf[64];
    bool is_null[8];
    unsigned long length[8];

    // id
    result_bind[0].buffer_type = MYSQL_TYPE_LONG;
    result_bind[0].buffer = &id;
    result_bind[0].is_null = &is_null[0];
    result_bind[0].length = &length[0];
    // user_id
    result_bind[1].buffer_type = MYSQL_TYPE_LONG;
    result_bind[1].buffer = &user_id_buf;
    result_bind[1].is_null = &is_null[1];
    result_bind[1].length = &length[1];
    // order_id
    result_bind[2].buffer_type = MYSQL_TYPE_LONG;
    result_bind[2].buffer = &order_id_buf;
    result_bind[2].is_null = &is_null[2];
    result_bind[2].length = &length[2];
    // cart_hash
    result_bind[3].buffer_type = MYSQL_TYPE_STRING;
    result_bind[3].buffer = cart_hash_buf;
    result_bind[3].buffer_length = sizeof(cart_hash_buf);
    result_bind[3].is_null = &is_null[3];
    result_bind[3].length = &length[3];
    // total
    result_bind[4].buffer_type = MYSQL_TYPE_DOUBLE;
    result_bind[4].buffer = &total;
    result_bind[4].is_null = &is_null[4];
    result_bind[4].length = &length[4];
    // idempotency_key
    result_bind[5].buffer_type = MYSQL_TYPE_STRING;
    result_bind[5].buffer = idemp_key_buf;
    result_bind[5].buffer_length = sizeof(idemp_key_buf);
    result_bind[5].is_null = &is_null[5];
    result_bind[5].length = &length[5];
    // paypal_order_id
    result_bind[6].buffer_type = MYSQL_TYPE_STRING;
    result_bind[6].buffer = paypal_order_buf;
    result_bind[6].buffer_length = sizeof(paypal_order_buf);
    result_bind[6].is_null = &is_null[6];
    result_bind[6].length = &length[6];
    // status
    result_bind[7].buffer_type = MYSQL_TYPE_STRING;
    result_bind[7].buffer = status_buf;
    result_bind[7].buffer_length = sizeof(status_buf);
    result_bind[7].is_null = &is_null[7];
    result_bind[7].length = &length[7];

    if (mysql_stmt_bind_result(stmt, result_bind) != 0)
    {
        std::cerr << "Failed to bind result: " << mysql_stmt_error(stmt) << std::endl;
        return {std::nullopt, Errors::BindResultFailed};
    }

    if (mysql_stmt_store_result(stmt) != 0)
    {
        std::cerr << "Failed to store result: " << mysql_stmt_error(stmt) << std::endl;
        return {std::nullopt, Errors::StoreResultFailed};
    }

    if (mysql_stmt_num_rows(stmt) == 0)
    {
        // No data
        return {std::vector<PaymentAttempt>{}, Errors::NoError};
    }

    std::vector<PaymentAttempt> attempts;
    while (true)
    {
        int fetch_result = mysql_stmt_fetch(stmt);
        if (fetch_result == MYSQL_NO_DATA)
            break;
        if (fetch_result != 0)
        {
            std::cerr << "Fetch error: " << mysql_stmt_error(stmt) << std::endl;
            return {std::nullopt, Errors::FetchFailed};
        }
        PaymentAttempt p;
        p.id = id;
        p.user_id = user_id_buf;
        p.order_id = order_id_buf;
        p.cart_hash = std::string(cart_hash_buf, length[3]);
        p.total = total;
        p.idempotency_key = std::string(idemp_key_buf, length[5]);
        p.paypal_order_id = std::string(paypal_order_buf, length[6]);
        p.status = std::string(status_buf, length[7]);
        attempts.push_back(std::move(p));
    }

    return {attempts, Errors::NoError};
}