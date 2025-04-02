#include "router/Router.h"
#include "server/Server.h"

Router::Router(web::http::experimental::listener::http_listener &listener, DatabaseConnection &db)
    : listener_(listener), db_(db) {}

void Router::setup_routes()
{

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
        } });
}
