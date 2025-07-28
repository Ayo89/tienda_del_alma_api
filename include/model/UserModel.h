#ifndef USERMODEL_H
#define USERMODEL_H

#include <optional>
#include <string>
#include <vector>
#include "AddressModel.h"
#include "entities/User.h"
#include "db/DatabaseConnection.h"
#include <array>
#include <cstring> // Para std::memset
#include <mysql/mysql.h>

class UserModel
{
public:
    UserModel();
    std::optional<int> createUser(const std::string &first_name, const std::string &password, const std::string &email, const std::string &auth_provider,const std::string &auth_id);
    std::optional<User> findUserById(int user_id);
    std::optional<User> findUserByEmail(const std::string &email);
    std::optional<User> findUserByEmailAndProvider(const std::string &email, const std::string &auth_provider);
};
#endif