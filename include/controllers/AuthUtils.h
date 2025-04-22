#pragma once
#include <cpprest/http_msg.h>
#include <optional>
#include <string>
#include "services/jwt/JwtService.h"

class AuthUtils
{
public:
    static std::optional<std::string> getUserIdFromRequest(const web::http::http_request &request);
};

