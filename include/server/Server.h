#ifndef SERVER_H
#define SERVER_H

#include <cpprest/http_listener.h>
#include "db/DatabaseConnection.h"
#include "router/Router.h"

using namespace web::http;
using namespace web::http::experimental::listener;

class Server
{
public:
    // Constructor que solo toma la dirección y la base de datos, sin iniciar nada
    Server(const utility::string_t &address, DatabaseConnection &db);

    // Métodos para controlar el servidor
    static void add_cors_headers(http_response &response);
    static void add_cookie(web::http::http_response &response, const std::string &cookie_value);
    void start();
    void stop();

private:
    web::http::experimental::listener::http_listener listener_;
    DatabaseConnection &db_;
    Router router_; // Agregamos el Router
};

#endif