#include "server/Server.h"
#include <iostream>

Server::Server(const utility::string_t &address, DatabaseConnection &db)
    : listener_(address), db_(db), router_(listener_, db_)
{
    // No se inicia nada aquí, solo se inicializan los miembros
}

void Server::start()
{
    router_.setup_routes();
    listener_.open()
        .then([]()
              { std::wcout << L"Servidor iniciado en http://localhost:8080" << std::endl; })
        .wait();
}

void Server::stop()
{
    listener_.close().wait();
}

void Server::add_cors_headers(http_response &response)
{
    response.headers().add(U("Access-Control-Allow-Origin"), U("http://localhost:5173")); // O usa un origen específico como U("http://localhost:3000")
    response.headers().add(U("Access-Control-Allow-Methods"), U("GET, POST, PUT, DELETE, OPTIONS"));
    response.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type, Authorization"));
    response.headers().add(U("Access-Control-Allow-Credentials"), U("true"));
}

void Server::add_cookie(http_response &response, const std::string &cookie_value)
{
    // Se arma la cookie. Aquí puedes personalizar las flags según lo necesites.
    std::string cookie = "token=" + cookie_value + ";   SameSite=none; Path=/";
    response.headers().add(U("Set-Cookie"), utility::conversions::to_string_t(cookie));
}