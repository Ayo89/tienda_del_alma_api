#include "controllers/UserController.h"
#include <iostream>

bool UserController::createUser(const std::string &name,
                                const std::string &password,
                                const std::string &email)
{
    bool success = model.createUser(name, password, email);
    if (success)
    {
        std::cout << "Usuario creado exitosamente: " << name << std::endl;
    }
    return success;
}