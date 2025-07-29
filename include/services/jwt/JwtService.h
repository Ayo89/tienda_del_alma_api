#ifndef JWT_SERVICE_H
#define JWT_SERVICE_H
#include <optional>
#include <string>
#include "entities/DecodedUser.h"

class JwtService
{
public:
    static std::string generateToken(const std::string &user_id, const std::string &email);
    static std::optional<std::string> decodeToken(const std::string &token);
    static std::optional<DecodedUser> verifyAndExtractUser(const std::string& token);   
};

#endif
