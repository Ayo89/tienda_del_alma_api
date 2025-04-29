#ifndef UTILSSOWNER_H
#define UTILSOWNER_H
#include <iostream>
#include <string>

class UtilsOwner
{
public:
    static auto base64_encode(const std::string &input) -> std::string;
};
#endif // UTILSSOWNER_H