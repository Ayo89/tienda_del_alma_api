// AuthUtils.h
#pragma once
#include <cpprest/http_msg.h>
#include <optional>
#include <string>

class AuthUtils
{
public:
    static std::optional<std::string> getUserIdFromRequest(const web::http::http_request &request);
};
