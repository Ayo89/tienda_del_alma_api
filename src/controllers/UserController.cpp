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

std::optional<User> UserController::getUserById(int user_id)
{
    auto userOpt = model.findUserById(user_id);
    if (userOpt)
    {
        std::cout << "Usuario encontrado: " << userOpt->first_name << std::endl;
    }
    else
    {
        std::cout << "Usuario con ID " << user_id << " no encontrado." << std::endl;
    }
    return userOpt;
}

std::optional<User> UserController::getUserByEmail(const std::string &email)
{
    auto userOpt = model.findUserByEmail(email);
    if (userOpt)
    {
        std::cout << "Email encontrado: " << userOpt->email << std::endl;
    }
    else
    {
        std::cout << "Email " << email << " no encontrado." << std::endl;
    }
    return userOpt;
}