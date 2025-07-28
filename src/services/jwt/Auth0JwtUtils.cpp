#include "services/jwt/Auth0JwtUtils.h"
#include <jwt-cpp/jwt.h>
#include <fstream>
#include <sstream>

std::string Auth0JwtUtils::readPemFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("No se pudo abrir el archivo PEM");
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

DecodedUser Auth0JwtUtils::verifyAndExtractUser(
    const std::string& token,
    const std::string& publicKeyPem,
    const std::string& expectedAudience,
    const std::string& expectedIssuer
) {
    auto decoded = jwt::decode(token);

    auto verifier = jwt::verify()
        .allow_algorithm(jwt::algorithm::rs256(publicKeyPem, "", "", ""))
        .with_issuer(expectedIssuer)
        .with_audience(expectedAudience);

    verifier.verify(decoded);

    DecodedUser user;
    user.sub = decoded.get_payload_claim("sub").as_string();
    if (decoded.has_payload_claim("email")) {
        user.email = decoded.get_payload_claim("email").as_string();
    }
    return user;
}
