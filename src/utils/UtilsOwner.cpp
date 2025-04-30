#include "utils/UtilsOwner.h"

auto UtilsOwner::base64_encode(const std::string &input) -> std::string
{
    static const char encode_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encoded;
    int val = 0, valb = -6;
    for (unsigned char c : input)
    {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0)
        {
            encoded.push_back(encode_table[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        encoded.push_back(encode_table[((val << 8) >> (valb + 8)) & 0x3F]);
    while (encoded.size() % 4)
        encoded.push_back('=');
    return encoded;
}

auto UtilsOwner::roundToTwoDecimal(double value) -> double
{
    return std::round(value * 100.0) / 100.0;
}

auto UtilsOwner::toString2Dec(double value) -> std::string
{
    std::ostringstream oss;
    oss << std::fixed
        << std::setprecision(2)
        << value;
    return oss.str();
}