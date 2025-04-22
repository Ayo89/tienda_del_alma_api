// AuthUtils.cpp
#include "controllers/AuthUtils.h"


std::optional<std::string> AuthUtils::getUserIdFromRequest(const web::http::http_request &request)
{
    auto cookies_header = request.headers().find(U("Cookie"));
    if (cookies_header != request.headers().end())
    {
        std::string cookies = utility::conversions::to_utf8string(cookies_header->second);
        size_t token_start = cookies.find("token=");
        if (token_start != std::string::npos)
        {
            token_start += 6;
            size_t token_end = cookies.find(";", token_start);
            if (token_end == std::string::npos)
            {
                token_end = cookies.length();
            }
            std::string token = cookies.substr(token_start, token_end - token_start);
            if (!token.empty())
            {
                return JwtService::decodeToken(token);
            }
        }
    }
    return std::nullopt;
}
