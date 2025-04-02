#include "db/DatabaseConnection.h"
#include <iostream>
#include "env/EnvLoader.h"

DatabaseConnection::DatabaseConnection()
    : connection(nullptr)
{
    // Cargar las variables del archivo .env directamente dentro del constructor
    EnvLoader env(".env");
    env.load(); // Cargar el archivo .env

    host = env.get("DB_HOST", "localhost");
    user = env.get("DB_USER", "root");
    password = env.get("DB_PASSWORD", "");
    dbname = env.get("DB_NAME", "tienda_del_alma");
    port = std::stoi(env.get("DB_PORT", "3306"));

    // Imprimir la contraseña para verificar que se cargó correctamente
    std::cerr << "Password: " << password << std::endl;
}

DatabaseConnection::DatabaseConnection(const std::string &host, const std::string &user, const std::string &password, const std::string &dbname, unsigned int port)
    : host(host), user(user), password(password), dbname(dbname), port(port), connection(nullptr) {}

DatabaseConnection::~DatabaseConnection()
{
    close();
}

bool DatabaseConnection::connect()
{
    connection = mysql_init(nullptr);
    if (connection == nullptr)
    {
        std::cerr << "Error al inicializar la conexión: " << mysql_error(connection) << std::endl;
        return false;
    }

    if (!mysql_real_connect(connection, host.c_str(), user.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0))
    {
        std::cerr << "Error al conectar a la base de datos: " << mysql_error(connection) << std::endl;
        mysql_close(connection);
        return false;
    }

    std::cout << "Conexión exitosa a la base de datos '" << dbname << "'." << std::endl;
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
    return connection;
}
