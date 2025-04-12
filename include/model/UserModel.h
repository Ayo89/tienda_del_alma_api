#ifndef USERMODEL_H
#define USERMODEL_H

#include <optional>
#include <string>
#include <vector>
#include "AddressModel.h"
#include "entities/User.h"
#include "db/DatabaseConnection.h"

class UserModel
{
private:
    DatabaseConnection &db; // Referencia almacenada
public:
    UserModel(DatabaseConnection &database) : db(database) {}
    std::optional<int> createUser(const std::string &first_name, const std::string &password, const std::string &email);
    std::optional<User> findUserById(int user_id);
    std::optional<User> findUserByEmail(const std::string &email);
};
#endif