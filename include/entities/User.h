#ifndef USER_H
#define USER_H

#include <string>

struct User
{
    int id;
    std::string first_name;
    std::string email;
    std::string password;
    std::string created_at;
};

#endif
