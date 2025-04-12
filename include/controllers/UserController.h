#ifndef USERCONTROLLER_H
#define USERCONTROLLER_H
#include <optional>
#include <string>
#include "db/DatabaseConnection.h"
#include "entities/User.h"
#include "model/UserModel.h"

class UserController
{
private:
    UserModel model;

public:
    UserController(DatabaseConnection &db) : model(db) {} // Pasamos db al constructor de UserModel
    std::optional<int> createUser(const std::string &first_name,
                    const std::string &password,
                    const std::string &email);

    std::optional<User> getUserById(int user_id);
    std::optional<User> getUserByEmail(const std::string &email);
};

#endif