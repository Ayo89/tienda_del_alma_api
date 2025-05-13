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