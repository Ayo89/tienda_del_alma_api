#ifndef ROUTER_H
#define ROUTER_H

#include <cpprest/http_listener.h>
#include "controllers/auth/AuthController.h"
#include "db/DatabaseConnection.h"

class Router
{
public:
    Router(web::http::experimental::listener::http_listener &listener, DatabaseConnection &db);
    void setup_routes();

private:
    web::http::experimental::listener::http_listener &listener_;
    DatabaseConnection &db_;
};

#endif
