#ifndef AUTH_MIDDLEWARE_H
#define AUTH_MIDDLEWARE_H

#include <cpprest/http_msg.h>
#include <optional>
#include <string>
#include "entities/DecodedUser.h"
#include "controllers/UserController.h"
#include "controllers/AuthUtils.h"


class AuthMiddleware {
public:
    static std::optional<DecodedUser> authenticateGoogleRequest(const web::http::http_request& request);
    static std::optional<DecodedUser> authenticateRequest(const web::http::http_request& request);
};

#endif // AUTH_MIDDLEWARE_H
