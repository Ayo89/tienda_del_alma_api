#pragma once
#include <string>
#include <map>
#include <mutex>
#include <chrono>

class JwksUtils {
public:
    static JwksUtils& getInstance();

    // Devuelve la clave pública en formato PEM para un KID
    std::string getPemForKid(const std::string& jwksUrl, const std::string& kid);

private:
    JwksUtils() = default;
    JwksUtils(const JwksUtils&) = delete;
    JwksUtils& operator=(const JwksUtils&) = delete;

    void refreshCache(const std::string& jwksUrl);

    std::map<std::string, std::string> kidToPemCache;
    std::chrono::system_clock::time_point lastFetchTime;
    std::mutex cacheMutex;
};
