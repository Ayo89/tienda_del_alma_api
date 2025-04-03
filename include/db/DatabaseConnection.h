// include/model/DatabaseConnection.h
#ifndef DATABASE_CONNECTION_H
#define DATABASE_CONNECTION_H

#include <mysql/mysql.h>
#include <string>
#include <stdexcept>

class DatabaseConnection
{
private:
    MYSQL *connection;
    std::string host;
    std::string user;
    std::string password;
    std::string dbname;
    unsigned int port;

public:
    DatabaseConnection(); // Constructor por defecto
    DatabaseConnection(const std::string &host, const std::string &user, const std::string &password, const std::string &dbname, unsigned int port);
    ~DatabaseConnection();

    bool connect();
    void close();
    MYSQL *getConnection();

};

#endif // DATABASE_CONNECTION_H
