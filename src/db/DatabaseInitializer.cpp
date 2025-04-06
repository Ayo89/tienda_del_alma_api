#include "db/DatabaseInitializer.h"
#include <iostream>

bool DatabaseInitializer::executeQuery(const std::string &query)
{
    MYSQL *conn = connection.getConnection();
    if (!conn)
    {
        std::cerr << "Conexión no establecida." << std::endl;
        return false;
    }

    // Ejecuta la consulta SQL. Si ocurre algún error, mysql_query retornará un valor distinto de 0.
    if (mysql_query(conn, query.c_str()))
    {
        std::cerr << "Error ejecutando consulta: " << mysql_error(conn) << std::endl;
        return false;
    }

    // Si no hubo errores, se retorna true.
    return true;
}

bool DatabaseInitializer::initialize(bool forceInit)
{
    MYSQL *conn = connection.getConnection();
    if (!conn)
    {
        std::cerr << "Conexión no establecida." << std::endl;
        return false;
    }

    if (forceInit)
    {
        const char *dropQueries[] = {
            "DROP TABLE IF EXISTS order_items;",
            "DROP TABLE IF EXISTS orders;",
            "DROP TABLE IF EXISTS inventory;",
            "DROP TABLE IF EXISTS products;",
            "DROP TABLE IF EXISTS categories;",
            "DROP TABLE IF EXISTS shipping_addresses;",
            "DROP TABLE IF EXISTS billing_addresses;",
            "DROP TABLE IF EXISTS password_resets;",
            "DROP TABLE IF EXISTS users;"};
        for (const char *q : dropQueries)
        {
            if (!executeQuery(std::string(q)))
            {
                std::cerr << "Error al ejecutar: " << q << std::endl;
            }
        }
    }

    std::string query;

    // Tabla de usuarios con solo nombre, email y contraseña
    query = "CREATE TABLE IF NOT EXISTS users ("
            "id INT AUTO_INCREMENT PRIMARY KEY, "
            "first_name VARCHAR(50) NOT NULL, "
            "email VARCHAR(100) NOT NULL UNIQUE, "
            "password VARCHAR(255) NOT NULL, "
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
            ") ENGINE=InnoDB;";
    if (!executeQuery(query))
        return false;

    query = "CREATE TABLE password_resets("
            "id SERIAL PRIMARY KEY, "
            "user_id INTEGER NOT NULL, "
            "token_hash TEXT NOT NULL, "
            "expires_at TIMESTAMP NOT NULL, "
            "FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE"
            ") ENGINE=InnoDB;";
    if (!executeQuery(query))
        return false;

    // Tabla de direcciones de envío con nombre, apellidos, teléfono y dirección
    query = "CREATE TABLE IF NOT EXISTS shipping_addresses ("
            "id INT AUTO_INCREMENT PRIMARY KEY, "
            "user_id INT NOT NULL, "
            "first_name VARCHAR(100) NOT NULL, "
            "last_name VARCHAR(100) NOT NULL, "
            "phone VARCHAR(20) NOT NULL, "
            "street VARCHAR(255) NOT NULL, "
            "city VARCHAR(100) NOT NULL, "
            "province VARCHAR(100), "
            "postal_code VARCHAR(20) NOT NULL, "
            "country VARCHAR(100) NOT NULL, "
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
            "FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE"
            ") ENGINE=InnoDB;";
    if (!executeQuery(query))
        return false;

    // Tabla de direcciones de facturación con nombre, apellidos, teléfono, dirección y DNI
    query = "CREATE TABLE IF NOT EXISTS billing_addresses ("
            "id INT AUTO_INCREMENT PRIMARY KEY, "
            "user_id INT NOT NULL, "
            "first_name VARCHAR(100) NOT NULL, "
            "last_name VARCHAR(100) NOT NULL, "
            "phone VARCHAR(20) NOT NULL, "
            "dni VARCHAR(50), "
            "street VARCHAR(255) NOT NULL, "
            "city VARCHAR(100) NOT NULL, "
            "province VARCHAR(100), "
            "postal_code VARCHAR(20) NOT NULL, "
            "country VARCHAR(100) NOT NULL, "
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
            "FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE"
            ") ENGINE=InnoDB;";
    if (!executeQuery(query))
        return false;

    // Tabla de categorías
    query = "CREATE TABLE IF NOT EXISTS categories ("
            "id INT AUTO_INCREMENT PRIMARY KEY, "
            "name VARCHAR(100) NOT NULL UNIQUE, "
            "description TEXT, "
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
            ") ENGINE=InnoDB;";
    if (!executeQuery(query))
        return false;

    // Tabla de productos con SKU (explicado más abajo)
    query = "CREATE TABLE IF NOT EXISTS products ("
            "id INT AUTO_INCREMENT PRIMARY KEY, "
            "sku VARCHAR(50) NOT NULL UNIQUE, "
            "name VARCHAR(100) NOT NULL, "
            "description TEXT, "
            "price DECIMAL(10,2) NOT NULL, "
            "category_id INT, "
            "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
            "FOREIGN KEY (category_id) REFERENCES categories(id) ON DELETE SET NULL"
            ") ENGINE=InnoDB;";
    if (!executeQuery(query))
        return false;

    // Tabla de inventario con restricción de cantidad
    query = "CREATE TABLE IF NOT EXISTS inventory ("
            "product_id INT PRIMARY KEY, "
            "quantity INT NOT NULL CHECK (quantity >= 0), "
            "last_updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, "
            "FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE CASCADE"
            ") ENGINE=InnoDB;";
    if (!executeQuery(query))
        return false;

    // Tabla de órdenes con campos de pago integrados
    query = "CREATE TABLE IF NOT EXISTS orders ("
            "id INT AUTO_INCREMENT PRIMARY KEY, "
            "user_id INT NOT NULL, "
            "order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
            "status VARCHAR(50) NOT NULL, "
            "total DECIMAL(10,2) NOT NULL, "
            "billing_address_id INT NOT NULL, "
            "shipping_address_id INT NOT NULL, "
            "shipment_date TIMESTAMP, "
            "carrier VARCHAR(100), "
            "tracking_number VARCHAR(100), "
            "payment_date TIMESTAMP, "
            "amount DECIMAL(10,2) NOT NULL, "
            "method VARCHAR(50), "
            "payment_status VARCHAR(50), "
            "FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE, "
            "FOREIGN KEY (billing_address_id) REFERENCES billing_addresses(id) ON DELETE CASCADE, "
            "FOREIGN KEY (shipping_address_id) REFERENCES shipping_addresses(id) ON DELETE CASCADE"
            ") ENGINE=InnoDB;";
    if (!executeQuery(query))
        return false;

    // Tabla de ítems de órdenes
    query = "CREATE TABLE IF NOT EXISTS order_items ("
            "id INT AUTO_INCREMENT PRIMARY KEY, "
            "order_id INT NOT NULL, "
            "product_id INT NOT NULL, "
            "quantity INT NOT NULL CHECK (quantity > 0), "
            "price DECIMAL(10,2) NOT NULL, "
            "FOREIGN KEY (order_id) REFERENCES orders(id) ON DELETE CASCADE, "
            "FOREIGN KEY (product_id) REFERENCES products(id)"
            ") ENGINE=InnoDB;";
    if (!executeQuery(query))
        return false;

    std::cout << "Base de datos inicializada correctamente con los cambios solicitados." << std::endl;
    return true;
}