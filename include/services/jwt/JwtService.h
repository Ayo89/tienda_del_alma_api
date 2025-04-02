#ifndef JWT_SERVICE_H
#define JWT_SERVICE_H

#include <string>

class JwtService
{
public:
    static std::string generateToken(const std::string &name, const std::string &email);
};

#endif
