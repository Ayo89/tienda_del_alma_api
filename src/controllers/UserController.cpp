#include "controllers/UserController.h"
#include <iostream>
bool UserController::createUser(const std::string &first_name,
                                const std::string &password,
                                const std::string &email)
{
    bool success = model.createUser(first_name, password, email);
    if (success)
    {
        std::cout << "Usuario creado exitosamente: " << first_name << std::endl;
    }
    return success;
}