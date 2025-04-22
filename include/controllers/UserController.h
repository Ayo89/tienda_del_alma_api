#ifndef USERCONTROLLER_H
#define USERCONTROLLER_H

#include <optional>
#include <string>
#include <iostream>
#include "entities/User.h"
#include "model/UserModel.h"

class UserController
{
private:
    UserModel model;

public:
    UserController();
    std::optional<int> createUser(const std::string &first_name,
                    const std::string &password,
                    const std::string &email);

    std::optional<User> getUserById(int user_id);
    std::optional<User> getUserByEmail(const std::string &email);
};

#endif