#include "model/AddressModel.h"
#include <mysql/mysql.h>
#include <cstring>
#include <iostream>

std::optional<int> AddressModel::createAddress(
    const int &user_id,
    const std::string &first_name,
    const std::string &last_name,
    const std::string &phone,
    const std::string &street,
    const std::string &city,
    const std::string &province,
    const std::string &postal_code,
    const std::string &country,
    const bool &is_default,
    const std::string &additional_info)
{
    // Validaciones básicas (additional_info es opcional)
    if (user_id == 0 || first_name.empty() || last_name.empty() || phone.empty() ||
        street.empty() || city.empty() || province.empty() || postal_code.empty() || country.empty())
    {
        std::cerr << "Error: Required fields cannot be empty" << std::endl;
        return std::nullopt;
    }

    // Obtener el puntero a la conexión MySQL
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Error: No active database connection" << std::endl;
        return std::nullopt;
    }

    // Preparar la consulta
    const char *query = "INSERT INTO shipping_addresses (user_id, first_name, last_name, phone, street, city, province, postal_code, country, is_default, additional_info) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Statement initialization failed: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cerr << "Statement preparation failed: " << mysql_stmt_error(stmt)
                  << " (MySQL error: " << mysql_error(conn) << ")" << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    // Vincular parámetros
    MYSQL_BIND bind[11];
    memset(bind, 0, sizeof(bind));

    // user_id
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (void *)&user_id;
    bind[0].is_unsigned = false;

    // first_name
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void *)first_name.c_str();
    bind[1].buffer_length = first_name.length();

    // last_name
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (void *)last_name.c_str();
    bind[2].buffer_length = last_name.length();

    // phone
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (void *)phone.c_str();
    bind[3].buffer_length = phone.length();

    // street
    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].buffer = (void *)street.c_str();
    bind[4].buffer_length = street.length();

    // city
    bind[5].buffer_type = MYSQL_TYPE_STRING;
    bind[5].buffer = (void *)city.c_str();
    bind[5].buffer_length = city.length();

    // province
    bind[6].buffer_type = MYSQL_TYPE_STRING;
    bind[6].buffer = (void *)province.c_str();
    bind[6].buffer_length = province.length();

    // postal_code
    bind[7].buffer_type = MYSQL_TYPE_STRING;
    bind[7].buffer = (void *)postal_code.c_str();
    bind[7].buffer_length = postal_code.length();

    // country
    bind[8].buffer_type = MYSQL_TYPE_STRING;
    bind[8].buffer = (void *)country.c_str();
    bind[8].buffer_length = country.length();

    // is_default
    bind[9].buffer_type = MYSQL_TYPE_TINY;
    bind[9].buffer = (void *)&is_default;
    bind[9].buffer_length = sizeof(is_default);

    // additional_info
    bind[10].buffer_type = MYSQL_TYPE_STRING;
    bind[10].buffer = (void *)additional_info.c_str();
    bind[10].buffer_length = additional_info.length();
    bind[10].is_null = additional_info.empty() ? new bool(true) : nullptr;

    // Vincular los parámetros
    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        std::cerr << "Parameter binding failed: " << mysql_stmt_error(stmt)
                  << " (MySQL error: " << mysql_error(conn) << ")" << std::endl;
        mysql_stmt_close(stmt);
        delete bind[10].is_null; // Limpieza
        return std::nullopt;
    }

    // Ejecutar la consulta preparada
    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Execution failed: " << mysql_stmt_error(stmt)
                  << " (MySQL error: " << mysql_error(conn) << ")" << std::endl;
        mysql_stmt_close(stmt);
        delete bind[10].is_null; // Limpieza
        return std::nullopt;
    }

    // Obtener el ID de la dirección creada
    int address_id = mysql_insert_id(conn);

    std::cout << "Address created successfully for user_id: " << user_id << std::endl;
    mysql_stmt_close(stmt);
    delete bind[10].is_null; // Limpieza
    return address_id;
}