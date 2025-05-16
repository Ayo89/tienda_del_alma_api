#include "model/CarrierModel.h"
#include <iostream>

CarrierModel::CarrierModel() {}

bool CarrierModel::insertSampleCarriers()
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Failed to get database connection" << std::endl;
        return false;
    }

    std::string query;

    // Insertar Correos
    query = "INSERT INTO carriers (name, price) VALUES ('Correos', 5.50);";
    if (mysql_query(conn, query.c_str()))
    {
        std::cerr << "Failed to insert Correos: " << mysql_error(conn) << std::endl;
        return false;
    }

    // Insertar Optitrans
    query = "INSERT INTO carriers (name, price) VALUES ('Optitrans', 7.25);";
    if (mysql_query(conn, query.c_str()))
    {
        std::cerr << "Failed to insert Optitrans: " << mysql_error(conn) << std::endl;
        return false;
    }

    // Insertar SEUR
    query = "INSERT INTO carriers (name, price) VALUES ('SEUR', 9.00);";
    if (mysql_query(conn, query.c_str()))
    {
        std::cerr << "Failed to insert SEUR: " << mysql_error(conn) << std::endl;
        return false;
    }

    std::cout << "Sample carriers inserted successfully." << std::endl;
    return true;
}

std::pair<std::optional<std::vector<Carrier>>, Errors> CarrierModel::getAllCarriers()
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Failed to get database connection" << std::endl;
        return {std::nullopt, Errors::DatabaseConnectionFailed};
    }

    const char *query = "SELECT id, name, price FROM carriers";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Failed to initialize statement" << std::endl;
        return {std::nullopt, Errors::StatementInitFailed};
    }
    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);
    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        std::cerr << "Failed to prepare statement: " << mysql_error(conn) << std::endl;
        return {std::nullopt, Errors::StatementPrepareFailed};
    }

    if (mysql_stmt_execute(stmt))
    {
        std::cerr << "Failed to execute statement: " << mysql_error(conn) << std::endl;
        return {std::nullopt, Errors::ExecutionFailed};
    }

    // BINDS RESULTS

    MYSQL_BIND result_bind[3];
    memset(result_bind, 0, sizeof(result_bind));

    int id;
    char name[50];
    double price;
    bool is_null[3];
    unsigned long length[3];

    result_bind[0].buffer_type = MYSQL_TYPE_LONG;
    result_bind[0].buffer = &id;
    result_bind[0].is_null = &is_null[0];
    result_bind[0].length = &length[0];

    result_bind[1].buffer_type = MYSQL_TYPE_STRING;
    result_bind[1].buffer = name;
    result_bind[1].buffer_length = 50;
    result_bind[1].is_null = &is_null[1];
    result_bind[1].length = &length[1];

    result_bind[2].buffer_type = MYSQL_TYPE_DOUBLE;
    result_bind[2].buffer = &price;
    result_bind[2].is_null = &is_null[2];
    result_bind[2].length = &length[2];

    if (mysql_stmt_bind_result(stmt, result_bind))
    {
        std::cerr << "Failed to bind result: " << mysql_error(conn) << std::endl;
        return {std::nullopt, Errors::BindParamFailed};
    }

    if (mysql_stmt_fetch(stmt) != 0)
    {
        std::cerr << "Fetch failed in Carrier Model -getAllCarriers- : " << mysql_stmt_error(stmt) << "\n";
        return {std::nullopt, Errors::FetchFailed};
    }
    Carrier carrier;
    std::vector<Carrier> carriers;
    do
    {
        carrier.id = id;
        carrier.name = name;
        carrier.price = price;
        carriers.push_back(carrier);
    } while (mysql_stmt_fetch(stmt) == 0);

    return {carriers, Errors::NoError};
}

std::pair<std::optional<Carrier>, Errors> CarrierModel::getCarrierById(int &id)
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Failed to get database connection" << std::endl;
        return {std::nullopt, Errors::DatabaseConnectionFailed};
    }

    const char *query = "SELECT id, name, price FROM carriers WHERE id = ?";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Failed to initialize statement" << std::endl;
        return {std::nullopt, Errors::StatementInitFailed};
    }
    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);
    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        std::cerr << "Failed to prepare statement: " << mysql_error(conn) << std::endl;
        return {std::nullopt, Errors::StatementPrepareFailed};
    }

    // BIND PARAMS
    MYSQL_BIND param_bind[1];
    memset(param_bind, 0, sizeof(param_bind));
    int id_param = id;
    param_bind[0].buffer_type = MYSQL_TYPE_LONG;
    param_bind[0].buffer = &id_param;
    param_bind[0].buffer_length = sizeof(id_param);

    if (mysql_stmt_bind_param(stmt, param_bind))
    {
        std::cerr << "Failed to bind param: " << mysql_error(conn) << std::endl;
        return {std::nullopt, Errors::BindParamFailed};
    }

    if (mysql_stmt_execute(stmt))
    {
        std::cerr << "Failed to execute statement: " << mysql_error(conn) << std::endl;
        return {std::nullopt, Errors::ExecutionFailed};
    }

    // BINDS RESULTS

    MYSQL_BIND result_bind[3];
    memset(result_bind, 0, sizeof(result_bind));

    int id_result;
    char name[50];
    double price;
    bool is_null[3];
    unsigned long length[3];

    result_bind[0].buffer_type = MYSQL_TYPE_LONG;
    result_bind[0].buffer = &id_result;
    result_bind[0].is_null = &is_null[0];
    result_bind[0].length = &length[0];

    result_bind[1].buffer_type = MYSQL_TYPE_STRING;
    result_bind[1].buffer = name;
    result_bind[1].buffer_length = 50;
    result_bind[1].is_null = &is_null[1];
    result_bind[1].length = &length[1];

    result_bind[2].buffer_type = MYSQL_TYPE_DOUBLE;
    result_bind[2].buffer = &price;
    result_bind[2].is_null = &is_null[2];
    result_bind[2].length = &length[2];

    if (mysql_stmt_bind_result(stmt, result_bind))
    {
        std::cerr << "Failed to bind result: " << mysql_error(conn) << std::endl;
        return {std::nullopt, Errors::BindParamFailed};
    }

    if (mysql_stmt_fetch(stmt) != 0)
    {
        std::cerr << "Fetch failed in Carrier Model -getCarrierById- : " << mysql_stmt_error(stmt) << "\n";
        return {std::nullopt, Errors::FetchFailed};
    }
    Carrier carrier;
    do
    {
        carrier.id = id_result;
        carrier.name = name;
        carrier.price = price;
    } while (mysql_stmt_fetch(stmt) == 0);
    std::cout << "Carrier found: " << carrier.name << " " << carrier.price << std::endl;
    return {carrier, Errors::NoError};
}