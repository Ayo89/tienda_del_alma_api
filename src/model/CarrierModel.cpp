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
