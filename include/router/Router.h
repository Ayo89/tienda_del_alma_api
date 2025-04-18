#ifndef ROUTER_H
#define ROUTER_H

#include <cpprest/http_listener.h>
#include "controllers/AuthController.h"
#include "db/DatabaseConnection.h"
#include "controllers/ProductController.h"
#include "controllers/UserController.h"
#include "controllers/AddressController.h"

class Router
{
public:
    Router(web::http::experimental::listener::http_listener &listener);
    void setup_routes();

private:
    web::http::experimental::listener::http_listener &listener_;
};

#endif
