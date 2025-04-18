#include "db/DatabaseConnection.h"
#include <iostream>
#include <stdexcept>
#include "env/EnvLoader.h"

DatabaseConnection::DatabaseConnection() : connection(nullptr)
{
    EnvLoader env(".env");
    env.load();

    host = env.get("DB_HOST", "localhost");
    user = env.get("DB_USER", "root");
    password = env.get("DB_PASSWORD", "");
    dbname = env.get("DB_NAME", "tienda_del_alma");

    try
    {
        port = static_cast<unsigned int>(std::stoi(env.get("DB_PORT", "3306")));
    }
    catch (...)
    {
        std::cerr << "Error al leer el puerto. Usando 3306." << std::endl;
        port = 3306;
    }

    connect();
}

DatabaseConnection::~DatabaseConnection()
{
    close();
}

DatabaseConnection &DatabaseConnection::getInstance()
{
    thread_local static DatabaseConnection instance;
    return instance;
}

bool DatabaseConnection::connect()
{
    if (connection)
    {
        mysql_close(connection);
        connection = nullptr;
    }

    connection = mysql_init(nullptr);
    if (!connection)
    {
        std::cerr << "mysql_init falló\n";
        return false;
    }

    if (!mysql_real_connect(connection, host.c_str(), user.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0))
    {
        std::cerr << "mysql_real_connect falló: " << mysql_error(connection) << std::endl;
        mysql_close(connection);
        connection = nullptr;
        return false;
    }

    return true;
}

MYSQL *DatabaseConnection::getConnection()
{
    if (!connection || mysql_ping(connection) != 0)
    {
        std::cerr << "[DB] Conexión inactiva, reconectando..." << std::endl;
        if (!connect())
        {
            std::cerr << "[DB] Error al reconectar." << std::endl;
            return nullptr;
        }
    }

    return connection;
}

void DatabaseConnection::close()
{
    if (connection)
    {
        mysql_close(connection);
        connection = nullptr;
        std::cout << "[DB] Conexión cerrada." << std::endl;
    }
}
