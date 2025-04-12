#include "services/jwt/JwtService.h"
#include <jwt-cpp/jwt.h>
#include "env/EnvLoader.h"

std::string JwtService::generateToken(const std::string &user_id, const std::string &email)
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
        .set_payload_claim("user_id", jwt::claim(user_id))
        .set_payload_claim("email", jwt::claim(email))
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(24))
        .sign(jwt::algorithm::hs256{secret});
}

std::optional<std::string> JwtService::decodeToken(const std::string &token)
{
    try
    {
        // Decodificar el token usando jwt-cpp
        auto decoded_token = jwt::decode(token);

        // Extraer el 'user_id' del payload utilizando el método adecuado
        if (decoded_token.has_payload_claim("user_id"))
        {
            // Acceder al claim 'user_id' y devolver su valor como string
            return decoded_token.get_payload_claim("user_id").as_string();
        }
        return std::nullopt; // Si no tiene 'user_id', retornamos nullopt
    }
    catch (const std::exception &e)
    {
        // Si ocurre algún error (token inválido o expirado), retornamos nullopt
        return std::nullopt;
    }
}