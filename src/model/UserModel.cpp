#include "model/UserModel.h"
#include <mysql/mysql.h>
#include <cstring>
#include <iostream>

UserModel::UserModel() {}

std::optional<int> UserModel::createUser(const std::string &first_name,
                                         const std::string &password,
                                         const std::string &email)
{
    // Validaciones básicas
    if (first_name.empty() || password.empty() || email.empty())
    {
        std::cerr << "Error: name, password, or email cannot be empty" << std::endl;
        return std::nullopt;
    }
    DatabaseConnection &db = DatabaseConnection::getInstance();
    // Obtener el puntero a la conexión MySQL
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Error: No active database connection" << std::endl;
        return std::nullopt;
    }

    // Preparar la consulta
    const char *query = "INSERT INTO users (first_name, password, email) VALUES (?, ?, ?)";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Statement initialization failed: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }
    // Preparar el statement
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cerr << "Statement preparation failed: " << mysql_stmt_error(stmt)
                  << " (MySQL error: " << mysql_error(conn) << ")" << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
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
        return std::nullopt;
    }

    // Ejecutar la consulta preparada
    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Execution failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    std::cout << "User created successfully: " << first_name << std::endl;

    int user_id = mysql_insert_id(conn);

    mysql_stmt_close(stmt);
    return user_id;
}

std::optional<User> UserModel::findUserById(int user_id)
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Error: No hay conexión activa a la base de datos." << std::endl;
        return std::nullopt;
    }

    const char *query = "SELECT * FROM users WHERE id = ?";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Error: Falló la inicialización del statement: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cerr << "Error: Falló la preparación del statement: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    // Vincular el parámetro de entrada (user_id)
    MYSQL_BIND param[1];
    memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_LONG;
    param[0].buffer = (char *)&user_id;
    param[0].is_unsigned = true;

    if (mysql_stmt_bind_param(stmt, param) != 0)
    {
        std::cerr << "Error: Falló al vincular el parámetro: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Error: Falló la ejecución: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    // Preparar para recibir resultados
    int id;
    char first_name[100];
    char email[100];
    unsigned long first_name_len, email_len;

    MYSQL_BIND result[3];
    memset(result, 0, sizeof(result));

    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = (char *)&id;

    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = first_name;
    result[1].buffer_length = sizeof(first_name);
    result[1].length = &first_name_len;

    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = email;
    result[2].buffer_length = sizeof(email);
    result[2].length = &email_len;

    if (mysql_stmt_bind_result(stmt, result) != 0)
    {
        std::cerr << "Error: Falló al vincular el resultado: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    // Almacenar el resultado
    if (mysql_stmt_store_result(stmt) != 0)
    {
        std::cerr << "Error: Falló al almacenar el resultado: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    // Obtener el primer (y único) registro
    if (mysql_stmt_fetch(stmt) != 0)
    {
        mysql_stmt_close(stmt);
        return std::nullopt; // No se encontró el usuario o se produjo un error
    }

    // Construir el objeto User a partir de los resultados
    User user;
    user.id = id;
    user.first_name = std::string(first_name, first_name_len);
    user.email = std::string(email, email_len);

    mysql_stmt_close(stmt);
    return user;
}

std::optional<User> UserModel::findUserByEmail(const std::string &email)
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Error: No hay conexión activa a la base de datos." << std::endl;
        return std::nullopt;
    }

    const char *query = "SELECT id, first_name, email, password, created_at FROM users WHERE email = ?";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Error: Falló la inicialización del statement: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cerr << "Error: Falló la preparación del statement: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    // Vincular el parámetro de entrada (email)
    MYSQL_BIND param[1];
    memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_STRING;
    param[0].buffer = (void *)email.c_str();
    param[0].buffer_length = email.length();

    if (mysql_stmt_bind_param(stmt, param) != 0)
    {
        std::cerr << "Error: Falló al vincular el parámetro: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Error: Falló la ejecución: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    // Preparar para recibir resultados
    int id;
    char first_name[100];
    char email_buf[100];
    char password[100];
    char created_at[100];
    unsigned long first_name_len, email_len, password_len, created_at_len;

    MYSQL_BIND result[5];
    memset(result, 0, sizeof(result));

    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = (char *)&id;

    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = first_name;
    result[1].buffer_length = sizeof(first_name);
    result[1].length = &first_name_len;

    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = email_buf;
    result[2].buffer_length = sizeof(email_buf);
    result[2].length = &email_len;

    result[3].buffer_type = MYSQL_TYPE_STRING;
    result[3].buffer = password;
    result[3].buffer_length = sizeof(password);
    result[3].length = &password_len;

    result[4].buffer_type = MYSQL_TYPE_STRING;
    result[4].buffer = created_at;
    result[4].buffer_length = sizeof(created_at);
    result[4].length = &created_at_len;

    if (mysql_stmt_bind_result(stmt, result) != 0)
    {
        std::cerr << "Error: Falló al vincular el resultado: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    if (mysql_stmt_store_result(stmt) != 0)
    {
        std::cerr << "Error: Falló al almacenar el resultado: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    if (mysql_stmt_fetch(stmt) != 0)
    {
        mysql_stmt_close(stmt);
        return std::nullopt; // No se encontró el usuario o se produjo un error
    }

    // Construir el objeto User a partir de los resultados
    User user;
    user.id = id;
    user.first_name = std::string(first_name, first_name_len);
    user.email = std::string(email_buf, email_len);
    user.password = std::string(password, password_len);
    user.created_at = std::string(created_at, created_at_len);

    mysql_stmt_close(stmt);
    return user;
}
