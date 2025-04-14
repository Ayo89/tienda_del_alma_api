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
    password = env.get("DB_PASSWORD", ""); // No imprimir la contraseña en producción
    dbname = env.get("DB_NAME", "tienda_del_alma");
    port = std::stoi(env.get("DB_PORT", "3306"));

// Si deseas imprimir información de debug, puedes habilitarlo con una bandera de compilación
#ifdef DEBUG
    std::cerr << "Conectando a la base de datos en " << host << ":" << port << " con el usuario " << user << std::endl;
#endif
}

DatabaseConnection::DatabaseConnection(const std::string &host, const std::string &user, const std::string &password, const std::string &dbname, unsigned int port)
    : host(host), user(user), password(password), dbname(dbname), port(port), connection(nullptr)
{
#ifdef DEBUG
    std::cerr << "Conectando a la base de datos en " << host << ":" << port << " con el usuario " << user << std::endl;
#endif
}

DatabaseConnection::~DatabaseConnection()
{
    close();
}

bool DatabaseConnection::connect()
{
    if (connection != nullptr)
    {
        // Verifica si la conexión sigue viva
        if (mysql_ping(connection) == 0)
        {
            std::cout << "Conexión ya activa." << std::endl;
            return true;
        }
        else
        {
            std::cerr << "Conexión inactiva o perdida. Reintentando reconexión..." << std::endl;

            // Cierra la conexión anterior antes de reiniciar
            mysql_close(connection);
            connection = nullptr;
        }
    }

    // Inicializa una nueva conexión
    connection = mysql_init(nullptr);
    if (connection == nullptr)
    {
        std::cerr << "Error al inicializar la conexión: " << mysql_error(connection) << std::endl;
        return false;
    }

    if (!mysql_real_connect(connection, host.c_str(), user.c_str(), password.c_str(), dbname.c_str(), port, nullptr, 0))
    {
        std::cerr << "Error al conectar a la base de datos: " << mysql_error(connection) << std::endl;
        mysql_close(connection); // Esto es seguro aquí porque fue recién creado
        connection = nullptr;
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
    // Si la conexión no está inicializada, intentar conectarse.
    if (!connection)
    {
        std::cerr << "No existe conexión activa. Intentando conectar..." << std::endl;
        if (!connect())
        {
            std::cerr << "Error al establecer conexión en getConnection()." << std::endl;
            return nullptr;
        }
    }
    else
    {
        // Verificar que la conexión esté activa mediante mysql_ping.
        if (mysql_ping(connection) != 0)
        {
            std::cerr << "Conexión inactiva o perdida. Reintentando reconexión..." << std::endl;
            close();
            if (!connect())
            {
                std::cerr << "Error al reconectar en getConnection()." << std::endl;
                return nullptr;
            }
        }
    }
    return connection;
}
