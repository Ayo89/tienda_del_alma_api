#ifndef DATABASECONNECTION_H
#define DATABASECONNECTION_H

#include <mysql/mysql.h>
#include <string>

class DatabaseConnection
{
public:
    DatabaseConnection();
    DatabaseConnection(const std::string &host,
                       const std::string &user,
                       const std::string &password,
                       const std::string &dbname,
                       unsigned int port);
    ~DatabaseConnection();

    // Evitar copias y moves accidentales
    DatabaseConnection(const DatabaseConnection &) = delete;
    DatabaseConnection &operator=(const DatabaseConnection &) = delete;
    DatabaseConnection(DatabaseConnection &&) = delete;
    DatabaseConnection &operator=(DatabaseConnection &&) = delete;

    bool connect();
    void close();
    MYSQL *getConnection();

private:
    MYSQL *connection;
    std::string host;
    std::string user;
    std::string password;
    std::string dbname;
    unsigned int port;
};

#endif // DATABASECONNECTION_H