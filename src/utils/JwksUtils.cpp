#include "utils/JwksUtils.h"
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <iostream>

using namespace web;
using namespace web::http;
using namespace web::http::client;

JwksUtils& JwksUtils::getInstance() {
    static JwksUtils instance;
    return instance;
}

std::string JwksUtils::getPemForKid(const std::string& jwksUrl, const std::string& kid) {
    std::lock_guard<std::mutex> lock(cacheMutex);

    // Refrescar cada 24h
    auto now = std::chrono::system_clock::now();
    if (kidToPemCache.empty() || now - lastFetchTime > std::chrono::hours(24)) {
        refreshCache(jwksUrl);
    }

    auto it = kidToPemCache.find(kid);
    if (it == kidToPemCache.end()) {
        throw std::runtime_error("KID no encontrado en JWKS");
    }

    return it->second;
}

void JwksUtils::refreshCache(const std::string& jwksUrl) {
    try {
        http_client client(utility::conversions::to_string_t(jwksUrl));
        http_response resp = client.request(methods::GET).get();

        if (resp.status_code() != status_codes::OK) {
            throw std::runtime_error("Error descargando JWKS: " + std::to_string(resp.status_code()));
        }

        auto jwks = resp.extract_json().get();
        kidToPemCache.clear();

        for (const auto& key : jwks.at(U("keys")).as_array()) {
            std::string kid = utility::conversions::to_utf8string(key.at(U("kid")).as_string());

            if (key.has_field(U("x5c"))) {
                auto x5c_array = key.at(U("x5c")).as_array();
                if (x5c_array.size() > 0) {  
                    std::string cert_b64 = utility::conversions::to_utf8string(x5c_array[0].as_string());
                    std::string pem = "-----BEGIN CERTIFICATE-----\n" + cert_b64 + "\n-----END CERTIFICATE-----\n";
                    kidToPemCache[kid] = pem;
                }
            }
        }

        lastFetchTime = std::chrono::system_clock::now();
        std::cout << "JWKS cache actualizado ✅" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error al refrescar JWKS: " << e.what() << std::endl;
        throw;
    }
}

