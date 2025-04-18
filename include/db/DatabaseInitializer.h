#ifndef DATABASEINITIALIZER_H
#define DATABASEINITIALIZER_H

#include "db/DatabaseConnection.h"

class DatabaseInitializer
{
public:
    bool initialize(bool forceInit = false);
    bool executeQuery(const std::string &query);

};

#endif