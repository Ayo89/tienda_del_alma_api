#include "middleware/AuthMiddleware.h"
#include "services/jwt/Auth0JwtUtils.h"
#include "env/EnvLoader.h"
#include "utils/JwksUtils.h"
#include <cpprest/http_msg.h>
#include <cpprest/json.h>
#include <iostream>
#include <jwt-cpp/jwt.h>

using namespace web;
using namespace web::http;

UserController userController;

std::optional<DecodedUser> AuthMiddleware::authenticateGoogleRequest(const http_request &request)
{
    try
    {
        EnvLoader env(".env");
        env.load();

        // 1. Leer el header Authorization: Bearer <access_token>
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

        std::string access_token = tokenStr.substr(7); // quitar "Bearer "

        // 2. Decodificar token para leer cabecera y claims
        // 2. Decodificar token para leer cabecera y claims
        auto decoded = jwt::decode(access_token);
        std::string kid = decoded.get_header_claim("kid").as_string();

        // 🔎 DEBUG: imprimir todos los claims
        auto payloadJson = decoded.get_payload_json();
        std::cout << "=== Claims del token ===" << std::endl;
        for (auto it = payloadJson.begin(); it != payloadJson.end(); ++it)
        {
            std::cout << it->first << " : " << it->second.to_str() << std::endl;
        }

        // 3. Obtener JWKS desde Auth0 (con cache)
        std::string jwksUrl =
            "https://" + env.get("AUTH0_DOMAIN") + "/.well-known/jwks.json";
        std::string publicKeyPem =
            JwksUtils::getInstance().getPemForKid(jwksUrl, kid);

        // 4. Verificar firma e issuer
        auto verifier = jwt::verify()
                            .allow_algorithm(jwt::algorithm::rs256(publicKeyPem, "", "", ""))
                            .with_issuer("https://" + env.get("AUTH0_DOMAIN") + "/")
                            .leeway(60);

        verifier.verify(decoded);

        // --- Validar audience manualmente (porque puede ser array)
        const auto audClaim = decoded.get_payload_claim("aud");
        const std::string expectedAud = env.get("AUTH0_AUDIENCE");

        bool audienceOk = false;

        if (audClaim.get_type() == jwt::json::type::array)
        {
            for (const auto &val : audClaim.as_array())
            {
                if (val.is<std::string>() && val.get<std::string>() == expectedAud)
                {
                    audienceOk = true;
                    break;
                }
            }
        }
        else if (audClaim.get_type() == jwt::json::type::string)
        {
            if (audClaim.as_string() == expectedAud)
            {
                audienceOk = true;
            }
        }

        if (!audienceOk)
        {
            throw std::runtime_error("token doesn't contain the required audience");
        }

        // 5. Extraer claims relevantes
        DecodedUser user;
        user.sub = decoded.get_payload_claim("sub").as_string();
        if (decoded.has_payload_claim("email"))
        {
            user.email = decoded.get_payload_claim("email").as_string();
        }

        // 6. Buscar en base de datos por auth_id (sub), ya que el access token no siempre trae email
        const auto dbUserOpt = userController.getUserByAuthId(user.sub);
        if (!dbUserOpt.has_value())
        {
            std::cerr << "Usuario no encontrado en la base de datos: " << user.sub << std::endl;
            return std::nullopt;
        }
        const auto &dbUser = dbUserOpt.value();
        user.id = dbUser.id;

        return user;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error autenticando token: " << ex.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<DecodedUser> AuthMiddleware::authenticateRequest(const http_request &request)
{
    auto headers = request.headers();

    if (!headers.has(U("Authorization")))
    {
        std::cerr << "Falta el header Authorization\n";
        return std::nullopt;
    }

    auto authHeader = utility::conversions::to_utf8string(headers[U("Authorization")]);

    if (authHeader.rfind("Bearer ", 0) != 0)
    {
        std::cerr << "Formato incorrecto del Authorization header\n";
        return std::nullopt;
    }

    std::string token = authHeader.substr(7);

    try
    {
        auto decoded = jwt::decode(token);

        if (!decoded.has_payload_claim("iss"))
        {
            std::cerr << "Token sin issuer\n";
            return std::nullopt;
        }

        std::string issuer = decoded.get_payload_claim("iss").as_string();

        EnvLoader env(".env");
        env.load();

        std::string auth0Issuer = "https://" + env.get("AUTH0_DOMAIN") + "/";

        if (issuer == auth0Issuer)
        {
            return authenticateGoogleRequest(request);
        }

        if (issuer == "tienda_del_alma")
        {
            return AuthUtils::getUserFromRequest(request);
        }

        std::cerr << "Issuer no reconocido: " << issuer << std::endl;
        return std::nullopt;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error autenticando token: " << ex.what() << std::endl;
        return std::nullopt;
    }
}
