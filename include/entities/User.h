#ifndef USER_H
#define USER_H

#include <string>

struct User
{
    int id;
    std::string first_name;
    std::string email;
    std::string password;
    std::string auth_provider; // "local" or "google"
    std::string auth_id; // ID from Google or empty for local users
    std::string created_at;
};

#endif
