
#include "db/DatabaseConnection.h"
#include <iostream>
#include "env/EnvLoader.h"

DatabaseConnection::DatabaseConnection()
    : connection(nullptr)
{
    EnvLoader env(".env");
    env.load();

    host = env.get("DB_HOST", "localhost");
    user = env.get("DB_USER", "root");
    password = env.get("DB_PASSWORD", "");
    dbname = env.get("DB_NAME", "tienda_del_alma");
    port = static_cast<unsigned int>(std::stoi(env.get("DB_PORT", "3306")));
}

DatabaseConnection::DatabaseConnection(const std::string &host,
                                       const std::string &user,
                                       const std::string &password,
                                       const std::string &dbname,
                                       unsigned int port)
    : connection(nullptr), host(host), user(user), password(password), dbname(dbname), port(port)
{
}

DatabaseConnection::~DatabaseConnection()
{
    close();
}

bool DatabaseConnection::connect()
{
    if (connection)
    {
        if (mysql_ping(connection) == 0)
        {
            std::cout << "Conexión ya activa." << std::endl;
            return true;
        }
        mysql_close(connection);
        connection = nullptr;
    }

    // Inicializa nueva conexión
    connection = mysql_init(nullptr);
    if (!connection)
    {
        std::cerr << "Error al inicializar la conexión: " << mysql_error(nullptr) << std::endl;
        return false;
    }

    // Reportar truncaciones de datos como errores
    bool report = 1;
    mysql_options(connection, MYSQL_REPORT_DATA_TRUNCATION, &report);

    if (!mysql_real_connect(connection,
                            host.c_str(),
                            user.c_str(),
                            password.c_str(),
                            dbname.c_str(),
                            port,
                            nullptr,
                            0))
    {
        std::cerr << "Error al conectar a la base de datos: " << mysql_error(connection) << std::endl;
        mysql_close(connection);
        connection = nullptr;
        return false;
    }

    std::cout << "Conexión exitosa a '" << dbname << "'." << std::endl;
    return true;
}

void DatabaseConnection::close()
{
    if (connection)
    {
        mysql_close(connection);
        connection = nullptr;
        std::cout << "Conexión cerrada." << std::endl;
    }
}

MYSQL *DatabaseConnection::getConnection()
{
    if (!connection)
    {
        std::cerr << "No existe conexión activa. Intentando conectar..." << std::endl;
        if (!connect())
        {
            std::cerr << "Error al establecer conexión en getConnection()." << std::endl;
            return nullptr;
        }
    }
    else if (mysql_ping(connection) != 0)
    {
        std::cerr << "Conexión inactiva o perdida. Reintentando reconexión..." << std::endl;
        close();
        if (!connect())
        {
            std::cerr << "Error al reconectar en getConnection()." << std::endl;
            return nullptr;
        }
    }
    return connection;
}
