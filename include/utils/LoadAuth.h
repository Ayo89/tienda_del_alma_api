#pragma once

#include <string>
#include <stdexcept>

// jwt-cpp
#include <jwt-cpp/jwt.h>

/// @brief Carga la clave pública desde el JWKS de Auth0 a partir del kid del JWT.
/// @param kid ID de clave pública (extraída del header del JWT)
/// @param auth0Domain Dominio de tu tenant de Auth0 (por ejemplo: "tu-tenant.auth0.com")
/// @return Clave pública tipo `rsa_public_key` para verificar el JWT
rsa_public_key loadAuth0PublicKey(const std::string& kid, const std::string& auth0Domain);