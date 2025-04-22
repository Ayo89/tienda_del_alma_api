#include "controllers/UserController.h"
UserController::UserController() : model() {}

std::optional<int> UserController::createUser(const std::string &first_name,
                                              const std::string &password,
                                              const std::string &email)
{
    std::optional<int> user_id = model.createUser(first_name, password, email);
    if (user_id.has_value())
    {
        std::cout << "Usuario creado exitosamente: " << first_name
                  << " con ID: " << user_id.value() << std::endl;
    }
    else
    {
        std::cerr << "Error al crear usuario: " << first_name << std::endl;
        return std::nullopt;
    }

    return user_id;
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