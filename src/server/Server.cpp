#include "server/Server.h"
#include "router/Router.h"
#include "db/DatabaseConnection.h"
#include <iostream>

Server::Server(const utility::string_t &address)
    : listener_(address), router_(listener_) {}

void Server::start()
{
    router_.setup_routes();

    listener_.open()
        .then([this]()
              {
                  std::cout << "Servidor iniciado en "
                            << listener_.uri().to_string()
                            << std::endl;
              })
        .wait();
}

void Server::stop()
{
    listener_.close().wait();
}

void Server::add_cors_headers(http_response &response)
{
    response.headers().add(U("Access-Control-Allow-Origin"), U("https://tiendadelalma.netlify.app"));
    response.headers().add(U("Access-Control-Allow-Methods"), U("GET, POST, PUT, DELETE, OPTIONS"));
    response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type, Authorization"));
    response.headers().add(U("Access-Control-Allow-Credentials"), U("true"));
}

void Server::add_cookie(http_response &response, const std::string &cookie_value)
{
    std::string cookie = "token=" + cookie_value + "; HttpOnly; SameSite=Lax; Path=/";
    response.headers().add(U("Set-Cookie"), utility::conversions::to_string_t(cookie));
}