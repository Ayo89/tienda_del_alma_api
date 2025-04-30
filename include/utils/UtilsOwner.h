#ifndef UTILSSOWNER_H
#define UTILSOWNER_H
#include <iostream>
#include <string>
#include <cmath>
#include <sstream>
#include <iomanip>

class UtilsOwner
{
public:
    static auto base64_encode(const std::string &input) -> std::string;
    static auto roundToTwoDecimal(double value) -> double;
    static auto toString2Dec(double value) -> std::string;
};
#endif // UTILSSOWNER_H