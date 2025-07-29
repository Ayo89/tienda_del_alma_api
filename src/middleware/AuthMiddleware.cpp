#include "middleware/AuthMiddleware.h"
#include "services/jwt/Auth0JwtUtils.h"
#include "env/EnvLoader.h"

#include <cpprest/http_msg.h>
#include <cpprest/json.h>
#include <iostream>

using namespace web;
using namespace web::http;

UserController userController;

std::optional<DecodedUser> AuthMiddleware::authenticateGoogleRequest(const http_request &request)
{
    try
    {
        EnvLoader env(".env");
        env.load();

        // Obtener token desde el header Authorization: Bearer <id_token>
        auto headers = request.headers();
        if (!headers.has(U("Authorization")))
        {
            std::cerr << "Falta el header Authorization\n";
            return std::nullopt;
        }
        auto authHeader = headers[U("Authorization")];
        auto tokenStr = utility::conversions::to_utf8string(authHeader);
        if (tokenStr.rfind("Bearer ", 0) != 0)
        {
            std::cerr << "Formato incorrecto del Authorization header\n";
            return std::nullopt;
        }

        std::string id_token = tokenStr.substr(7); // Quitar "Bearer "

        // Leer clave pública del archivo
        std::string publicKeyPem = Auth0JwtUtils::readPemFile("config/auth0_public.pem");

        // Verificar y extraer los datos
        auto decoded = Auth0JwtUtils::verifyAndExtractUser(
            id_token,
            publicKeyPem,
            env.get("GOOGLE_CLIENT_ID"),
            env.get("AUTH0_ISSUER"));
        const auto dbUserOpt = userController.getUserByEmail(decoded.email);
        if (!dbUserOpt.has_value())
        {
            std::cerr << "Usuario no encontrado en la base de datos: " << decoded.email << std::endl;
            return std::nullopt;
        }
        const auto &dbUser = dbUserOpt.value();
        DecodedUser enriched;
        enriched.id = dbUser.id; // Aquí pones el user_id real
        enriched.email = decoded.email;
        enriched.sub = decoded.sub;

        return enriched;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error autenticando token: " << ex.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<DecodedUser> AuthMiddleware::authenticateRequest(const http_request &request)
{
    auto userOptGoogle = authenticateGoogleRequest(request);
    if (userOptGoogle.has_value())
    {
        return userOptGoogle;
    }

    auto userOptLocal = AuthUtils::getUserFromRequest(request);
    if (userOptLocal.has_value())
    {
        return userOptLocal;
    }

    return std::nullopt;
}