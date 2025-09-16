#ifndef ROUTER_H
#define ROUTER_H

#include <cpprest/http_listener.h>
#include "db/DatabaseConnection.h"
#include "entities/DecodedUser.h"
#include "controllers/AuthController.h"
#include "controllers/ProductController.h"
#include "controllers/UserController.h"
#include "controllers/AddressController.h"
#include "controllers/OrderController.h"
#include "controllers/PaypalController.h"
#include "controllers/CarrierController.h"
#include "controllers/CategoryController.h"
#include "controllers/InventoryController.h"
#include "middleware/AuthMiddleware.h"


class Router
{
public:
    Router(web::http::experimental::listener::http_listener &listener);
    void setup_routes();

private:
    web::http::experimental::listener::http_listener &listener_;
};

#endif
