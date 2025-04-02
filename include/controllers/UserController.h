#ifndef USERCONTROLLER_H
#define USERCONTROLLER_H

#include "UserModel.h"
#include "AddressModel.h"
#include <iostream>
#include <vector>
#include "model/DatabaseConnection.h" // Ajustado para tu estructura

class UserController
{
private:
    static bool createUser(const std::string &username,
                           const std::string &password,
                           const std::string &email,
                           DatabaseConnection &db);
};

#endif
