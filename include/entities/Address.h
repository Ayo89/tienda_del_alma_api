#ifndef ADDRESS_H
#define ADDRESS_H

#include <string>

struct Address
{
    int id;
    std::string first_name;
    std::string last_name;
    std::string phone;
    std::string street;
    std::string city;
    std::string province;
    std::string postal_code;
    std::string country;
    bool is_default;
    std::string additional_info;
    std::string type; // "billing" o "shipping"
};

#endif
