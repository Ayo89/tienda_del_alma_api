#ifndef USERMODEL_H
#define USERMODEL_H

#include <string>
#include <vector>
#include "AddressModel.h"
#include "db/DatabaseConnection.h" // Ajustado para tu estructura

class UserModel
{
public:
    static bool createUser(const std::string &name,
                           const std::string &password,
                           const std::string &email,
                           DatabaseConnection &db);
};

#endif
