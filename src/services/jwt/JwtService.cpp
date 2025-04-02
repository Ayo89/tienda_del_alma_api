#include "services/jwt/JwtService.h"
#include <jwt-cpp/jwt.h>
#include "env/EnvLoader.h"

std::string JwtService::generateToken(const std::string &name, const std::string &email)
{
    EnvLoader env(".env");
    env.load();

    std::string secret = env.get("JWT_SECRET", "");
    if (secret.empty())
    {
        throw std::runtime_error("JWT_SECRET no está configurado");
    }

    return jwt::create()
        .set_issuer("tienda_del_alma")
        .set_payload_claim("name", jwt::claim(name))
        .set_payload_claim("email", jwt::claim(email))
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(24))
        .sign(jwt::algorithm::hs256{secret});
}
