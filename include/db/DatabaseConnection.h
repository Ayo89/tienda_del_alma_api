#ifndef DATABASECONNECTION_H
#define DATABASECONNECTION_H

#pragma once
#include <mysql/mysql.h>
#include <string>

class DatabaseConnection
{
public:
    static DatabaseConnection &getInstance();

    MYSQL *getConnection();
    void close();

private:
    DatabaseConnection();
    ~DatabaseConnection();

    bool connect();

    MYSQL *connection;

    // Parámetros de conexión
    std::string host;
    std::string user;
    std::string password;
    std::string dbname;
    unsigned int port;
};

#endif