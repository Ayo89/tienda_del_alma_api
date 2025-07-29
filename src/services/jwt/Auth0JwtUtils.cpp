#include "services/jwt/Auth0JwtUtils.h"
#include <jwt-cpp/jwt.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <ctime>

std::string Auth0JwtUtils::readPemFile(const std::string& path) {
    std::cout << "[readPemFile] Intentando abrir archivo PEM: " << path << std::endl;
    std::ifstream in(path);
    if (!in) {
        std::cerr << "[readPemFile] Error: No se pudo abrir el archivo PEM" << std::endl;
        throw std::runtime_error("No se pudo abrir el archivo PEM");
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::cout << "[readPemFile] Archivo PEM leído correctamente" << std::endl;
    return buffer.str();
}

DecodedUser Auth0JwtUtils::verifyAndExtractUser(
    const std::string& token,
    const std::string& publicKeyPem,
    const std::string& expectedAudience,
    const std::string& expectedIssuer
) {
    std::cout << "[verifyAndExtractUser] Iniciando verificación del token..." << std::endl;

    // Decodificar token sin verificar aún
    std::cout << "[verifyAndExtractUser] Decodificando token..." << std::endl;
    auto decoded = jwt::decode(token);
    std::cout << "[verifyAndExtractUser] Token decodificado con éxito" << std::endl;

    // Obtener claims para validaciones manuales
    const auto& iat = decoded.get_payload_claim("iat").as_date();
    const auto& exp = decoded.get_payload_claim("exp").as_date();
    auto now = std::chrono::system_clock::now();

    std::time_t iat_time = std::chrono::system_clock::to_time_t(iat);
    std::time_t exp_time = std::chrono::system_clock::to_time_t(exp);
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    std::cout << "🕒 [Token Timing] iat: " << iat_time 
              << " | exp: " << exp_time 
              << " | now: " << now_time 
              << " | diff exp-now: " << (exp_time - now_time) << std::endl;

    if (exp < now) {
        throw std::runtime_error("token expired");
    }

    // Verificación de firma, issuer y audiencia (sin `exp`)
    std::cout << "[verifyAndExtractUser] Configurando verificador con issuer: "
              << expectedIssuer << " y audience: " << expectedAudience << std::endl;

    auto verifier = jwt::verify()
        .allow_algorithm(jwt::algorithm::rs256(publicKeyPem, "", "", ""))
        .with_issuer(expectedIssuer)
        .with_audience(expectedAudience)
        .leeway(60); // tolerancia opcional de 60 segundos

    std::cout << "[verifyAndExtractUser] Verificando token..." << std::endl;
    verifier.verify(decoded);
    std::cout << "[verifyAndExtractUser] Token verificado correctamente" << std::endl;

    // Extraer datos
    DecodedUser user;
    user.sub = decoded.get_payload_claim("sub").as_string();
    std::cout << "[verifyAndExtractUser] sub: " << user.sub << std::endl;

    if (decoded.has_payload_claim("email")) {
        user.email = decoded.get_payload_claim("email").as_string();
        std::cout << "[verifyAndExtractUser] email: " << user.email << std::endl;
    } else {
        std::cout << "[verifyAndExtractUser] No se encontró el campo 'email' en el token" << std::endl;
    }

    std::cout << "[verifyAndExtractUser] Extracción de usuario completa" << std::endl;
    return user;
}
