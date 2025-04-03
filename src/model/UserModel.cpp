#include "model/UserModel.h"
#include <mysql/mysql.h>
#include <cstring>
#include <iostream>

bool UserModel::createUser(const std::string &first_name,
                           const std::string &password,
                           const std::string &email)
{
    // Validaciones básicas
    if (first_name.empty() || password.empty() || email.empty())
    {
        std::cerr << "Error: name, password, or email cannot be empty" << std::endl;
        return false;
    }

    // Obtener el puntero a la conexión MySQL
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Error: No active database connection" << std::endl;
        return false;
    }
    std::cout << "first name: " << first_name << std::endl;
    // Preparar la consulta
    const char *query = "INSERT INTO users (first_name, password, email) VALUES (?, ?, ?)";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Statement initialization failed: " << mysql_error(conn) << std::endl;
        return false;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cerr << "Statement preparation failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }

    // Vincular parámetros
    MYSQL_BIND bind[3];
    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void *)first_name.c_str();
    bind[0].buffer_length = first_name.length();

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void *)password.c_str();
    bind[1].buffer_length = password.length();

    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (void *)email.c_str();
    bind[2].buffer_length = email.length();

    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        std::cerr << "Parameter binding failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }

    // Ejecutar la consulta preparada
    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Execution failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return false;
    }

    std::cout << "User created successfully: " << first_name << std::endl;
    mysql_stmt_close(stmt);
    return true;
}