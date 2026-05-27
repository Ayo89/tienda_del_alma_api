#pragma once

#include <string>

struct DecodedUser
{
    int id = 0;
    std::string sub;
    std::string email;
    std::string issuer;
    bool email_verified = false;
    std::string name;
    std::string given_name;
    std::string family_name;
    std::string picture;
    std::string locale;
};