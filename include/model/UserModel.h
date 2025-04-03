#ifndef USERMODEL_H
#define USERMODEL_H

#include <string>
#include <vector>
#include "AddressModel.h"
#include "db/DatabaseConnection.h"

class UserModel
{
private:
    DatabaseConnection &db; // Referencia almacenada
public:
    UserModel(DatabaseConnection &database) : db(database) {}
    bool createUser(const std::string &first_name, const std::string &password, const std::string &email);
};
#endif