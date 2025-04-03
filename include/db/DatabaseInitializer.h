#include "db/DatabaseConnection.h"

class DatabaseInitializer
{
public:
    DatabaseInitializer(DatabaseConnection &db) : connection(db) {}
    bool initialize(bool forceInit = false);
    bool executeQuery(const std::string &query);

private:
    DatabaseConnection &connection;
};
