#pragma once
#include <string>

struct DecodedUser {
    std::string sub;
    std::string email;
};

class Auth0JwtUtils {
public:
    static DecodedUser verifyAndExtractUser(
        const std::string& token,
        const std::string& publicKeyPem,
        const std::string& expectedAudience,
        const std::string& expectedIssuer
    );

    static std::string readPemFile(const std::string& path);
};
