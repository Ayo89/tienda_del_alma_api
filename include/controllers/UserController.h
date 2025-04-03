#ifndef USERCONTROLLER_H
#define USERCONTROLLER_H

#include "model/UserModel.h"
#include <string>

class UserController
{
private:
    UserModel model;

public:
    UserController(DatabaseConnection &db) : model(db) {} // Pasamos db al constructor de UserModel
    bool createUser(const std::string &first_name,
                    const std::string &password,
                    const std::string &email);
};

#endif