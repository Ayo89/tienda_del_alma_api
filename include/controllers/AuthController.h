#ifndef AUTHROUTER_H
#define AUTHROUTER_H

#include <cpprest/http_msg.h>
#include <cpprest/json.h>
#include <string>
#include "db/DatabaseConnection.h"
#include "controllers/UserController.h"

class AuthController
{

private:
    UserController userController;

public:
    AuthController();
    web::http::http_response signup(const web::http::http_request &request);
    web::http::http_response login(const web::http::http_request &request);
};

#endif // AUTHROUTER_H