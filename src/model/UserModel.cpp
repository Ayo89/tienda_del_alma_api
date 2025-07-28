#include "model/UserModel.h"
#include <mysql/mysql.h>
#include <cstring>
#include <iostream>

UserModel::UserModel() {}

std::optional<int> UserModel::createUser(const std::string &first_name,
                                         const std::string &password,
                                         const std::string &email,
                                         const std::string &auth_provider,
                                         const std::string &auth_id)
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Error: No DB connection" << std::endl;
        return std::nullopt;
    }

    const char *query = "INSERT INTO users (first_name, password, email, auth_provider, auth_id) VALUES (?, ?, ?, ?, ?)";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt || mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cerr << "Statement error: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }

    MYSQL_BIND bind[5];
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

    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (void *)auth_provider.c_str();
    bind[3].buffer_length = auth_provider.length();

    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].buffer = (void *)auth_id.c_str();
    bind[4].buffer_length = auth_id.length();

    if (mysql_stmt_bind_param(stmt, bind) != 0 || mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Execution failed: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

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

    const char *query = "SELECT id, first_name, email, password, auth_provider, auth_id, created_at FROM users WHERE email = ?";
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
    char auth_provider[100];
    char auth_id[255];
    unsigned long first_name_len, email_len, password_len, auth_provider_len, auth_id_len, created_at_len;
    bool is_null[7];
    MYSQL_BIND result[7];
    memset(result, 0, sizeof(result));

    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = (char *)&id;
    result[0].is_null = &is_null[0];

    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = first_name;
    result[1].buffer_length = sizeof(first_name);
    result[1].length = &first_name_len;
    result[1].is_null = &is_null[1];

    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = email_buf;
    result[2].buffer_length = sizeof(email_buf);
    result[2].length = &email_len;
    result[2].is_null = &is_null[2];

    result[3].buffer_type = MYSQL_TYPE_STRING;
    result[3].buffer = password;
    result[3].buffer_length = sizeof(password);
    result[3].length = &password_len;
    result[3].is_null = &is_null[3];

    result[4].buffer_type = MYSQL_TYPE_STRING;
    result[4].buffer = auth_provider;
    result[4].buffer_length = sizeof(auth_provider);
    result[4].length = &auth_provider_len;
    result[4].is_null = &is_null[4];

    result[5].buffer_type = MYSQL_TYPE_STRING;
    result[5].buffer = auth_id;
    result[5].buffer_length = sizeof(auth_id);
    result[5].length = &auth_id_len;
    result[5].is_null = &is_null[5];

        result[6].buffer_type = MYSQL_TYPE_STRING;
    result[6].buffer = created_at;
    result[6].buffer_length = sizeof(created_at);
    result[6].length = &created_at_len;
    result[6].is_null = &is_null[6];

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
    user.auth_provider = std::string(auth_provider, auth_provider_len);
    user.auth_id = std::string(auth_id, auth_id_len);
    user.created_at = std::string(created_at, created_at_len);

    mysql_stmt_close(stmt);
    return user;
}

std::optional<User> UserModel::findUserByEmailAndProvider(const std::string &email, const std::string &auth_provider)
{
    DatabaseConnection &db = DatabaseConnection::getInstance();
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Error: No DB connection" << std::endl;
        return std::nullopt;
    }

    const char *query = "SELECT id, first_name, email, password, auth_provider, auth_id, created_at FROM users WHERE email = ? AND auth_provider = ?";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt || mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cerr << "Statement error: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }

    MYSQL_BIND param[2];
    memset(param, 0, sizeof(param));

    param[0].buffer_type = MYSQL_TYPE_STRING;
    param[0].buffer = (void *)email.c_str();
    param[0].buffer_length = email.length();

    param[1].buffer_type = MYSQL_TYPE_STRING;
    param[1].buffer = (void *)auth_provider.c_str();
    param[1].buffer_length = auth_provider.length();

    if (mysql_stmt_bind_param(stmt, param) != 0 || mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Param bind or execute error: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    // Campos de salida
    int id;
    char first_name[100], email_buf[100], password[100], provider[50], auth_id[255], created_at[100];
    unsigned long first_name_len, email_len, password_len, provider_len, auth_id_len, created_at_len;

    MYSQL_BIND result[7];
    memset(result, 0, sizeof(result));

    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &id;

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
    result[4].buffer = provider;
    result[4].buffer_length = sizeof(provider);
    result[4].length = &provider_len;

    result[5].buffer_type = MYSQL_TYPE_STRING;
    result[5].buffer = auth_id;
    result[5].buffer_length = sizeof(auth_id);
    result[5].length = &auth_id_len;

    result[6].buffer_type = MYSQL_TYPE_STRING;
    result[6].buffer = created_at;
    result[6].buffer_length = sizeof(created_at);
    result[6].length = &created_at_len;

    if (mysql_stmt_bind_result(stmt, result) != 0 || mysql_stmt_store_result(stmt) != 0 || mysql_stmt_fetch(stmt) != 0)
    {
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    User user;
    user.id = id;
    user.first_name = std::string(first_name, first_name_len);
    user.email = std::string(email_buf, email_len);
    user.password = std::string(password, password_len);
    user.auth_provider = std::string(provider, provider_len);
    user.auth_id = std::string(auth_id, auth_id_len);
    user.created_at = std::string(created_at, created_at_len);

    mysql_stmt_close(stmt);
    return user;
}