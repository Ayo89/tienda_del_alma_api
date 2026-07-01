#include "model/PaymentAttemptModel.h"

std::pair<std::optional<PaymentAttempt>, Errors> PaymentAttempModel::createPaymentAttempt(
    int user_id,
    int order_id,
    std::string cart_hash,
    double total,
    std::string idempotency_key,
    std::string paypal_order_id,
    std::string status,
    std::string reserved_items_json)
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Failed to get database connection" << std::endl;
        return std::make_pair(std::nullopt, Errors::DatabaseConnectionFailed);
    }

    const char *query =
        "INSERT INTO payment_attempts "
        "(user_id, order_id, cart_hash, total, idempotency_key, paypal_order_id, status, reserved_items) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

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

    MYSQL_BIND bind[8];
    memset(bind, 0, sizeof(bind));

    unsigned long len_cart_hash = cart_hash.size();
    unsigned long len_idempotency_key = idempotency_key.size();
    unsigned long len_paypal_order_id = paypal_order_id.size();
    unsigned long len_status = status.size();
    unsigned long len_reserved_items = reserved_items_json.size();

    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = &user_id;

    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &order_id;

    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (char *)cart_hash.c_str();
    bind[2].buffer_length = len_cart_hash;
    bind[2].length = &len_cart_hash;

    bind[3].buffer_type = MYSQL_TYPE_DOUBLE;
    bind[3].buffer = &total;

    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].buffer = (char *)idempotency_key.c_str();
    bind[4].buffer_length = len_idempotency_key;
    bind[4].length = &len_idempotency_key;

    bind[5].buffer_type = MYSQL_TYPE_STRING;
    bind[5].buffer = (char *)paypal_order_id.c_str();
    bind[5].buffer_length = len_paypal_order_id;
    bind[5].length = &len_paypal_order_id;

    bind[6].buffer_type = MYSQL_TYPE_STRING;
    bind[6].buffer = (char *)status.c_str();
    bind[6].buffer_length = len_status;
    bind[6].length = &len_status;

    bind[7].buffer_type = MYSQL_TYPE_STRING;
    bind[7].buffer = (char *)reserved_items_json.c_str();
    bind[7].buffer_length = len_reserved_items;
    bind[7].length = &len_reserved_items;

    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        std::cerr << "Failed to bind parameters in createPaymentAttempt: " << mysql_error(conn) << std::endl;
        return std::make_pair(std::nullopt, Errors::BindParamFailed);
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Failed to execute statement in createPaymentAttempt: " << mysql_error(conn) << std::endl;
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
    paymentAttempt.reserved_items_json = reserved_items_json;

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

    // id
    result_bind[0].buffer_type = MYSQL_TYPE_LONG;
    result_bind[0].buffer = &id;
    result_bind[0].buffer_length = sizeof(id);
    result_bind[0].is_null = &is_null[0];

    // user_id
    result_bind[1].buffer_type = MYSQL_TYPE_LONG;
    result_bind[1].buffer = &user_id_buf;
    result_bind[1].buffer_length = sizeof(user_id_buf);
    result_bind[1].is_null = &is_null[1];

    // order_id
    result_bind[2].buffer_type = MYSQL_TYPE_LONG;
    result_bind[2].buffer = &order_id_buf;
    result_bind[2].buffer_length = sizeof(order_id_buf);
    result_bind[2].is_null = &is_null[2];

    // cart_hash
    result_bind[3].buffer_type = MYSQL_TYPE_STRING;
    result_bind[3].buffer = cart_hash_buf;
    result_bind[3].buffer_length = sizeof(cart_hash_buf);
    result_bind[3].is_null = &is_null[3];

    // total
    result_bind[4].buffer_type = MYSQL_TYPE_DOUBLE;
    result_bind[4].buffer = &total;
    result_bind[4].buffer_length = sizeof(total);
    result_bind[4].is_null = &is_null[4];

    // idempotency_key
    result_bind[5].buffer_type = MYSQL_TYPE_STRING;
    result_bind[5].buffer = idemp_key_buf;
    result_bind[5].buffer_length = sizeof(idemp_key_buf);
    result_bind[5].is_null = &is_null[5];

    // paypal_order_id
    result_bind[6].buffer_type = MYSQL_TYPE_STRING;
    result_bind[6].buffer = paypal_order_buf;
    result_bind[6].buffer_length = sizeof(paypal_order_buf);
    result_bind[6].is_null = &is_null[6];

    // status
    result_bind[7].buffer_type = MYSQL_TYPE_STRING;
    result_bind[7].buffer = status_buf;
    result_bind[7].buffer_length = sizeof(status_buf);
    result_bind[7].is_null = &is_null[7];

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
        p.cart_hash = std::string(cart_hash_buf, sizeof(cart_hash_buf));
        p.total = total;
        p.idempotency_key = std::string(idemp_key_buf, sizeof(idemp_key_buf));
        p.paypal_order_id = std::string(paypal_order_buf, sizeof(paypal_order_buf));
        p.status = std::string(status_buf, sizeof(status_buf));
        attempts.push_back(std::move(p));
    }

    return {attempts, Errors::NoError};
}

std::pair<bool, Errors> PaymentAttempModel::updatePaymentAttemptStatus(
    std::string &paypal_order_id,
    int order_id,
    int user_id,
    const std::string &status,
    const std::string &new_paypal_order_id)
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0)
    {
        std::cerr << "Failed to get database connection" << std::endl;
        return std::make_pair(false, Errors::DatabaseConnectionFailed);
    }

    try
    {
        if (mysql_query(conn, "START TRANSACTION") != 0)
        {
            std::cerr << "Failed to start transaction" << std::endl;
            return std::make_pair(false, Errors::TransactionStartFailed);
        }

        bool updatePaypalId = !new_paypal_order_id.empty();

        const char *query = updatePaypalId
                                ? "UPDATE payment_attempts SET status = ?, paypal_order_id = ?, created_at = CURRENT_TIMESTAMP WHERE paypal_order_id = ? AND order_id = ? AND user_id = ?"
                                : "UPDATE payment_attempts SET status = ? WHERE paypal_order_id = ? AND order_id = ? AND user_id = ?";

        MYSQL_STMT *stmt = mysql_stmt_init(conn);
        if (!stmt)
        {
            std::cerr << "Failed to initialize statement" << std::endl;
            mysql_query(conn, "ROLLBACK");
            return std::make_pair(false, Errors::StatementInitFailed);
        }
        auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);

        if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
        {
            std::cerr << "Failed to prepare statement: " << mysql_stmt_error(stmt) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return std::make_pair(false, Errors::StatementPrepareFailed);
        }

        unsigned long status_len = status.size();
        unsigned long old_paypal_len = paypal_order_id.size();
        unsigned long new_paypal_len = new_paypal_order_id.size();

        if (updatePaypalId)
        {
            MYSQL_BIND bind[5];
            memset(bind, 0, sizeof(bind));

            bind[0].buffer_type = MYSQL_TYPE_STRING;
            bind[0].buffer = (char *)status.c_str();
            bind[0].buffer_length = status_len;
            bind[0].length = &status_len;

            bind[1].buffer_type = MYSQL_TYPE_STRING;
            bind[1].buffer = (char *)new_paypal_order_id.c_str();
            bind[1].buffer_length = new_paypal_len;
            bind[1].length = &new_paypal_len;

            bind[2].buffer_type = MYSQL_TYPE_STRING;
            bind[2].buffer = (char *)paypal_order_id.c_str();
            bind[2].buffer_length = old_paypal_len;
            bind[2].length = &old_paypal_len;

            bind[3].buffer_type = MYSQL_TYPE_LONG;
            bind[3].buffer = (char *)&order_id;
            bind[3].buffer_length = sizeof(order_id);

            bind[4].buffer_type = MYSQL_TYPE_LONG;
            bind[4].buffer = (char *)&user_id;
            bind[4].buffer_length = sizeof(user_id);

            if (mysql_stmt_bind_param(stmt, bind) != 0)
            {
                std::cerr << "Failed to bind parameters: " << mysql_stmt_error(stmt) << std::endl;
                mysql_query(conn, "ROLLBACK");
                return std::make_pair(false, Errors::BindParamFailed);
            }
        }
        else
        {
            MYSQL_BIND bind[4];
            memset(bind, 0, sizeof(bind));

            bind[0].buffer_type = MYSQL_TYPE_STRING;
            bind[0].buffer = (char *)status.c_str();
            bind[0].buffer_length = status_len;
            bind[0].length = &status_len;

            bind[1].buffer_type = MYSQL_TYPE_STRING;
            bind[1].buffer = (char *)paypal_order_id.c_str();
            bind[1].buffer_length = old_paypal_len;
            bind[1].length = &old_paypal_len;

            bind[2].buffer_type = MYSQL_TYPE_LONG;
            bind[2].buffer = (char *)&order_id;
            bind[2].buffer_length = sizeof(order_id);

            bind[3].buffer_type = MYSQL_TYPE_LONG;
            bind[3].buffer = (char *)&user_id;
            bind[3].buffer_length = sizeof(user_id);

            if (mysql_stmt_bind_param(stmt, bind) != 0)
            {
                std::cerr << "Failed to bind parameters: " << mysql_stmt_error(stmt) << std::endl;
                mysql_query(conn, "ROLLBACK");
                return std::make_pair(false, Errors::BindParamFailed);
            }
        }

        if (mysql_stmt_execute(stmt) != 0)
        {
            std::cerr << "Failed to execute statement: " << mysql_stmt_error(stmt) << std::endl;
            mysql_query(conn, "ROLLBACK");
            return std::make_pair(false, Errors::ExecutionFailed);
        }

        if (mysql_stmt_affected_rows(stmt) == 0)
        {
            mysql_query(conn, "ROLLBACK");
            return std::make_pair(true, Errors::NoRowsAffected);
        }

        if (mysql_query(conn, "COMMIT") != 0)
        {
            std::cerr << "Failed to commit transaction: " << mysql_error(conn) << std::endl;
            return {false, Errors::CommitFailed};
        }

        return std::make_pair(true, Errors::NoError);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        mysql_query(conn, "ROLLBACK");
        return std::make_pair(false, Errors::UnknownError);
    }
}

std::pair<std::optional<std::vector<PaymentAttempt>>, Errors> PaymentAttempModel::getExpiredPendingAttempts(int minutesThreshold)
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Failed to get database connection" << std::endl;
        return {std::nullopt, Errors::DatabaseConnectionFailed};
    }

    const char *query =
        "SELECT id, user_id, order_id, cart_hash, total, idempotency_key, paypal_order_id, status, reserved_items"
        "  FROM payment_attempts"
        " WHERE status = 'PENDING'"
        "   AND created_at < NOW() - INTERVAL ? MINUTE";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
        return {std::nullopt, Errors::StatementInitFailed};
    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
        return {std::nullopt, Errors::StatementPrepareFailed};

    MYSQL_BIND bind_param[1];
    memset(bind_param, 0, sizeof(bind_param));
    bind_param[0].buffer_type = MYSQL_TYPE_LONG;
    bind_param[0].buffer = &minutesThreshold;
    bind_param[0].buffer_length = sizeof(minutesThreshold);
    if (mysql_stmt_bind_param(stmt, bind_param) != 0)
        return {std::nullopt, Errors::BindParamFailed};

    if (mysql_stmt_execute(stmt) != 0)
        return {std::nullopt, Errors::ExecutionFailed};

    MYSQL_BIND result_bind[9];
    memset(result_bind, 0, sizeof(result_bind));

    int id, user_id_buf, order_id_buf;
    double total;
    char cart_hash_buf[256], idemp_key_buf[256], paypal_order_buf[256], status_buf[64];
    char reserved_items_buf[4096];
    bool is_null[9];
    unsigned long lengths[9];

    result_bind[0] = {};
    result_bind[0].buffer_type = MYSQL_TYPE_LONG;
    result_bind[0].buffer = &id;
    result_bind[0].is_null = &is_null[0];
    result_bind[0].length = &lengths[0];
    result_bind[1] = {};
    result_bind[1].buffer_type = MYSQL_TYPE_LONG;
    result_bind[1].buffer = &user_id_buf;
    result_bind[1].is_null = &is_null[1];
    result_bind[1].length = &lengths[1];
    result_bind[2] = {};
    result_bind[2].buffer_type = MYSQL_TYPE_LONG;
    result_bind[2].buffer = &order_id_buf;
    result_bind[2].is_null = &is_null[2];
    result_bind[2].length = &lengths[2];
    result_bind[3] = {};
    result_bind[3].buffer_type = MYSQL_TYPE_STRING;
    result_bind[3].buffer = cart_hash_buf;
    result_bind[3].buffer_length = sizeof(cart_hash_buf);
    result_bind[3].is_null = &is_null[3];
    result_bind[3].length = &lengths[3];
    result_bind[4] = {};
    result_bind[4].buffer_type = MYSQL_TYPE_DOUBLE;
    result_bind[4].buffer = &total;
    result_bind[4].is_null = &is_null[4];
    result_bind[4].length = &lengths[4];
    result_bind[5] = {};
    result_bind[5].buffer_type = MYSQL_TYPE_STRING;
    result_bind[5].buffer = idemp_key_buf;
    result_bind[5].buffer_length = sizeof(idemp_key_buf);
    result_bind[5].is_null = &is_null[5];
    result_bind[5].length = &lengths[5];
    result_bind[6] = {};
    result_bind[6].buffer_type = MYSQL_TYPE_STRING;
    result_bind[6].buffer = paypal_order_buf;
    result_bind[6].buffer_length = sizeof(paypal_order_buf);
    result_bind[6].is_null = &is_null[6];
    result_bind[6].length = &lengths[6];
    result_bind[7] = {};
    result_bind[7].buffer_type = MYSQL_TYPE_STRING;
    result_bind[7].buffer = status_buf;
    result_bind[7].buffer_length = sizeof(status_buf);
    result_bind[7].is_null = &is_null[7];
    result_bind[7].length = &lengths[7];
    result_bind[8] = {};
    result_bind[8].buffer_type = MYSQL_TYPE_STRING;
    result_bind[8].buffer = reserved_items_buf;
    result_bind[8].buffer_length = sizeof(reserved_items_buf);
    result_bind[8].is_null = &is_null[8];
    result_bind[8].length = &lengths[8];

    if (mysql_stmt_bind_result(stmt, result_bind) != 0)
        return {std::nullopt, Errors::BindResultFailed};

    if (mysql_stmt_store_result(stmt) != 0)
        return {std::nullopt, Errors::StoreResultFailed};

    std::vector<PaymentAttempt> attempts;
    while (true)
    {
        int fetch_result = mysql_stmt_fetch(stmt);
        if (fetch_result == MYSQL_NO_DATA)
            break;
        if (fetch_result != 0)
            return {std::nullopt, Errors::FetchFailed};

        PaymentAttempt p;
        p.id = id;
        p.user_id = user_id_buf;
        p.order_id = order_id_buf;
        p.cart_hash = std::string(cart_hash_buf, lengths[3]);
        p.total = total;
        p.idempotency_key = std::string(idemp_key_buf, lengths[5]);
        p.paypal_order_id = std::string(paypal_order_buf, lengths[6]);
        p.status = std::string(status_buf, lengths[7]);
        p.reserved_items_json = is_null[8] ? "" : std::string(reserved_items_buf, lengths[8]);
        attempts.push_back(std::move(p));
    }

    return {attempts, Errors::NoError};
}

std::pair<std::optional<std::vector<PaymentAttempt>>, Errors> PaymentAttempModel::getPendingAttemptsByOrderId(int order_id)
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Failed to get database connection" << std::endl;
        return {std::nullopt, Errors::DatabaseConnectionFailed};
    }

    const char *query =
        "SELECT id, user_id, order_id, cart_hash, total, idempotency_key, paypal_order_id, status, reserved_items"
        "  FROM payment_attempts"
        " WHERE order_id = ? AND status IN ('PENDING','CANCELLED','EXPIRED')";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
        return {std::nullopt, Errors::StatementInitFailed};
    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
        return {std::nullopt, Errors::StatementPrepareFailed};

    MYSQL_BIND bind_param[1];
    memset(bind_param, 0, sizeof(bind_param));
    bind_param[0].buffer_type = MYSQL_TYPE_LONG;
    bind_param[0].buffer = &order_id;
    bind_param[0].buffer_length = sizeof(order_id);
    if (mysql_stmt_bind_param(stmt, bind_param) != 0)
        return {std::nullopt, Errors::BindParamFailed};

    if (mysql_stmt_execute(stmt) != 0)
        return {std::nullopt, Errors::ExecutionFailed};

    MYSQL_BIND result_bind[9];
    memset(result_bind, 0, sizeof(result_bind));

    int id, user_id_buf, order_id_buf;
    double total;
    char cart_hash_buf[256], idemp_key_buf[256], paypal_order_buf[256], status_buf[64];
    char reserved_items_buf[4096];
    bool is_null[9];
    unsigned long lengths[9];

    result_bind[0] = {};
    result_bind[0].buffer_type = MYSQL_TYPE_LONG;
    result_bind[0].buffer = &id;
    result_bind[0].is_null = &is_null[0];
    result_bind[0].length = &lengths[0];
    result_bind[1] = {};
    result_bind[1].buffer_type = MYSQL_TYPE_LONG;
    result_bind[1].buffer = &user_id_buf;
    result_bind[1].is_null = &is_null[1];
    result_bind[1].length = &lengths[1];
    result_bind[2] = {};
    result_bind[2].buffer_type = MYSQL_TYPE_LONG;
    result_bind[2].buffer = &order_id_buf;
    result_bind[2].is_null = &is_null[2];
    result_bind[2].length = &lengths[2];
    result_bind[3] = {};
    result_bind[3].buffer_type = MYSQL_TYPE_STRING;
    result_bind[3].buffer = cart_hash_buf;
    result_bind[3].buffer_length = sizeof(cart_hash_buf);
    result_bind[3].is_null = &is_null[3];
    result_bind[3].length = &lengths[3];
    result_bind[4] = {};
    result_bind[4].buffer_type = MYSQL_TYPE_DOUBLE;
    result_bind[4].buffer = &total;
    result_bind[4].is_null = &is_null[4];
    result_bind[4].length = &lengths[4];
    result_bind[5] = {};
    result_bind[5].buffer_type = MYSQL_TYPE_STRING;
    result_bind[5].buffer = idemp_key_buf;
    result_bind[5].buffer_length = sizeof(idemp_key_buf);
    result_bind[5].is_null = &is_null[5];
    result_bind[5].length = &lengths[5];
    result_bind[6] = {};
    result_bind[6].buffer_type = MYSQL_TYPE_STRING;
    result_bind[6].buffer = paypal_order_buf;
    result_bind[6].buffer_length = sizeof(paypal_order_buf);
    result_bind[6].is_null = &is_null[6];
    result_bind[6].length = &lengths[6];
    result_bind[7] = {};
    result_bind[7].buffer_type = MYSQL_TYPE_STRING;
    result_bind[7].buffer = status_buf;
    result_bind[7].buffer_length = sizeof(status_buf);
    result_bind[7].is_null = &is_null[7];
    result_bind[7].length = &lengths[7];
    result_bind[8] = {};
    result_bind[8].buffer_type = MYSQL_TYPE_STRING;
    result_bind[8].buffer = reserved_items_buf;
    result_bind[8].buffer_length = sizeof(reserved_items_buf);
    result_bind[8].is_null = &is_null[8];
    result_bind[8].length = &lengths[8];

    if (mysql_stmt_bind_result(stmt, result_bind) != 0)
        return {std::nullopt, Errors::BindResultFailed};

    if (mysql_stmt_store_result(stmt) != 0)
        return {std::nullopt, Errors::StoreResultFailed};

    std::vector<PaymentAttempt> attempts;
    while (true)
    {
        int fetch_result = mysql_stmt_fetch(stmt);
        if (fetch_result == MYSQL_NO_DATA)
            break;
        if (fetch_result != 0)
            return {std::nullopt, Errors::FetchFailed};

        PaymentAttempt p;
        p.id = id;
        p.user_id = user_id_buf;
        p.order_id = order_id_buf;
        p.cart_hash = std::string(cart_hash_buf, lengths[3]);
        p.total = total;
        p.idempotency_key = std::string(idemp_key_buf, lengths[5]);
        p.paypal_order_id = std::string(paypal_order_buf, lengths[6]);
        p.status = std::string(status_buf, lengths[7]);
        p.reserved_items_json = is_null[8] ? "" : std::string(reserved_items_buf, lengths[8]);
        attempts.push_back(std::move(p));
    }

    return {attempts, Errors::NoError};
}