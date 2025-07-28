#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H

#include "controllers/UserController.h"
#include "services/jwt/Auth0JwtUtils.h"
#include <cpprest/http_msg.h>
#include <cpprest/json.h>
#include <string>
#include <sodium.h>
#include "env/EnvLoader.h"
#include "services/jwt/JwtService.h"
#include <future> // Para usar std::async
#include <cpprest/http_client.h>
#include <cpprest/uri.h> 

class AuthController
{

private:
    UserController userController;

public:
    AuthController();
    web::http::http_response signup(const web::http::http_request &request);
    web::http::http_response login(const web::http::http_request &request);
    web::http::http_response googleLogin(const web::http::http_request &request);
};

#endif 