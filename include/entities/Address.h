#ifndef ADDRESS_H
#define ADDRESS_H

#include <string>

struct Address
{
    int id;
    int user_id;
    std::string first_name;
    std::string last_name;
    std::string phone;
    std::string street;
    std::string city;
    std::string province;
    std::string postal_code;
    std::string country;
};

#endif
