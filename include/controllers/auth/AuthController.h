#ifndef AUTHROUTER_H
#define AUTHROUTER_H

#include <cpprest/http_msg.h>
#include <cpprest/json.h>
#include <string>
#include "db/DatabaseConnection.h"

class AuthController
{
public:
    static web::http::http_response signup(const web::http::http_request &request, DatabaseConnection &db);
};

#endif // AUTHROUTER_H