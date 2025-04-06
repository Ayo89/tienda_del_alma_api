#include "router/Router.h"
#include "server/Server.h"

Router::Router(web::http::experimental::listener::http_listener &listener, DatabaseConnection &db)
    : listener_(listener), db_(db) {}

void Router::setup_routes()
{
    // Manejar solicitudes OPTIONS (preflight) para CORS
    listener_.support(web::http::methods::OPTIONS, [this](const web::http::http_request &request)
                      {
        web::http::http_response response(web::http::status_codes::OK);
        Server::add_cors_headers(response);  // Añadir encabezados CORS para OPTIONS
        request.reply(response); });

    // Manejar todas las solicitudes con soporte para POST, GET, etc.
    listener_.support([this](const web::http::http_request &request)
                      {
        auto path = request.relative_uri().path();
        auto method = request.method();
        web::http::http_response response(web::http::status_codes::NotFound);
        response.set_body(U("Ruta no encontrada"));

        // Procesar según el método y la ruta
        if (method == web::http::methods::POST && path == U("/signup"))
        {
            response = AuthController::signup(request, db_);
        }
        else if (method == web::http::methods::POST && path == U("/login"))
        {
            response = AuthController::login(request, db_); 
        }
        else if (method == web::http::methods::GET && path == U("/users"))
        {
            /* response = UserController::getUsers(db_); */
        }

        // 🔹 Añadir encabezados CORS
        Server::add_cors_headers(response);

        // Si el token está en los encabezados, lo agregamos como una cookie
        if (response.headers().has(U("X-Token")))
        {
            auto token = response.headers()[U("X-Token")];
            Server::add_cookie(response, utility::conversions::to_utf8string(token));
            response.headers().remove(U("X-Token"));  // Eliminamos el header para no exponer el token
        }

        // Responder a la solicitud con los encabezados CORS ya añadidos
        request.reply(response); });
}

/*
    // Manejar solicitudes OPTIONS (preflight)
    listener_.support(web::http::methods::OPTIONS, [this](const web::http::http_request &request)
                      {
        web::http::http_response response(web::http::status_codes::OK);
        Server::add_cors_headers(response); // Añadir encabezados CORS
        request.reply(response); });

    listener_.support(web::http::methods::POST, [this](const web::http::http_request &request)
                      {
        if (request.relative_uri().path() == U("/signup")) {
            web::http::http_response response = AuthController::signup(request, db_);
            Server::add_cors_headers(response); // Añades los encabezados CORS
            request.reply(response);
        } else {
            web::http::http_response not_found(web::http::status_codes::NotFound);
            not_found.set_body(U("Ruta no encontrada"));
            Server::add_cors_headers(not_found); // Añades los encabezados CORS
            request.reply(not_found);
        } }); */
