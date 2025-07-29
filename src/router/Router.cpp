
#include "router/Router.h"
#include "server/Server.h"
#include "middleware/AuthMiddleware.h"

AuthController authController;
AddressController addressController;
OrderController orderController;

Router::Router(web::http::experimental::listener::http_listener &listener)
    : listener_(listener) {}

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
        //USERS
        // Procesar según el método y la ruta
        if (method == web::http::methods::POST && path == U("/signup"))
        {
            response = authController.signup(request);
        }
        else if (method == web::http::methods::POST && path == U("/login"))
        {
            response = authController.login(request);
        }

        //GOOGLE
        if (method == web::http::methods::POST && path == U("/auth-google"))
        {
            response = authController.googleLogin(request);
        }



        //PRODUCTS

        if(method == web::http::methods::GET && path == U("/products"))
        {
            ProductController model;
            response = model.getAllProducts();
        }

        // SHIPPING ADDRESS
        auto segments_addresses = web::uri::split_path(request.request_uri().path());
        if (method == web::http::methods::GET && path == U("/address"))
        {
            auto userOpt = AuthMiddleware::authenticateRequest(request);
            if (!userOpt.has_value()) {
                response.set_status_code(status_codes::Unauthorized);
                response.set_body(json::value::object({ {U("error"), json::value::string(U("No autorizado"))} }));
                return response;
            }
            DecodedUser user = userOpt.value();
            response = addressController.getAddressesByUserId(request, user.id);
        } 
        else if (method == web::http::methods::GET && path.find(U("/address/")) != std::string::npos)
        {
                        auto userOpt = AuthMiddleware::authenticateRequest(request);
            if (!userOpt.has_value()) {
                response.set_status_code(status_codes::Unauthorized);
                response.set_body(json::value::object({ {U("error"), json::value::string(U("No autorizado"))} }));
                return response;
            }
            DecodedUser user = userOpt.value();
            response = addressController.getAddressById(request, user.id);
        }
        else if (method == web::http::methods::POST && path == U("/address"))
        {
            auto userOpt = AuthMiddleware::authenticateRequest(request);
            if (!userOpt.has_value()) {
                response.set_status_code(status_codes::Unauthorized);
                response.set_body(json::value::object({ {U("error"), json::value::string(U("No autorizado"))} }));
                return response;
            }
            
            DecodedUser user = userOpt.value();
            response = addressController.createAddress(request, user.id);
        }
        else if (method == web::http::methods::PUT && path.find(U("/address/default")) != std::string::npos)
        {
             auto userOpt = AuthMiddleware::authenticateRequest(request);
            if (!userOpt.has_value()) {
                response.set_status_code(status_codes::Unauthorized);
                response.set_body(json::value::object({ {U("error"), json::value::string(U("No autorizado"))} }));
                return response;
            }

            DecodedUser user = userOpt.value();
            response = addressController.setDefaultAddressController(request, user.id);
        }
        
        else if(method == web::http::methods::PUT && path.find(U("/address")) != std::string::npos)
        {
             auto userOpt = AuthMiddleware::authenticateRequest(request);
            if (!userOpt.has_value()) {
                response.set_status_code(status_codes::Unauthorized);
                response.set_body(json::value::object({ {U("error"), json::value::string(U("No autorizado"))} }));
                return response;
            }
            
            DecodedUser user = userOpt.value();
            response = addressController.updateAddress(request, user.id);
        }
        else if(method == web::http::methods::DEL && path.find(U("/address/")) != std::string::npos)
        {
            auto userOpt = AuthMiddleware::authenticateRequest(request);
            if (!userOpt.has_value()) {
                response.set_status_code(status_codes::Unauthorized);
                response.set_body(json::value::object({ {U("error"), json::value::string(U("No autorizado"))} }));
                return response;
            }

            DecodedUser user = userOpt.value();
            response = addressController.deleteAddress(request, user.id);
        }

        // ORDERS

        if (method == web::http::methods::POST && path == U("/order"))
        {
             auto userOpt = AuthMiddleware::authenticateRequest(request);
            if (!userOpt.has_value()) {
                response.set_status_code(status_codes::Unauthorized);
                response.set_body(json::value::object({ {U("error"), json::value::string(U("No autorizado"))} }));
                return response;
            }

            DecodedUser user = userOpt.value();
            response = orderController.createOrder(request, user.id);

        } 
        else if (method == web::http::methods::GET && path == U("/order"))
        {
             auto userOpt = AuthMiddleware::authenticateRequest(request);
            if (!userOpt.has_value()) {
                response.set_status_code(status_codes::Unauthorized);
                response.set_body(json::value::object({ {U("error"), json::value::string(U("No autorizado"))} }));
                return response;
            }

            DecodedUser user = userOpt.value();
            response = orderController.getOrdersByUserId(request, user.id);
        } 
        else if(method == web::http::methods::GET && path.find(U("/order/")) != std::string::npos)
        {
             auto userOpt = AuthMiddleware::authenticateRequest(request);
            if (!userOpt.has_value()) {
                response.set_status_code(status_codes::Unauthorized);
                response.set_body(json::value::object({ {U("error"), json::value::string(U("No autorizado"))} }));
                return response;
            }

            DecodedUser user = userOpt.value();
            response = orderController.getOrderById(request, user.id);
        }
        else if (method == web::http::methods::PUT)
        {
             auto userOpt = AuthMiddleware::authenticateRequest(request);
            if (!userOpt.has_value()) {
                response.set_status_code(status_codes::Unauthorized);
                response.set_body(json::value::object({ {U("error"), json::value::string(U("No autorizado"))} }));
                return response;
            }

            DecodedUser user = userOpt.value();
            auto path_segments = web::uri::split_path(request.request_uri().path());

            if (path_segments.size() == 4 &&
                path_segments[0] == U("order") &&
                path_segments[2] == U("carrier"))
            {
                response = orderController.updateTotalbyOrderId(request, user.id);
            }
        }

        //PAYPAL
        PaypalController paypalController;
        auto segments = web::uri::split_path(request.request_uri().path());
        
        if (method == methods::POST && segments.size() == 3 || segments.size() == 4 && segments[0] == U("paypal"))
        {
            // POST /paypal/{orderId}/create
            if (segments[2] == U("create"))
            {
             auto userOpt = AuthMiddleware::authenticateRequest(request);
            if (!userOpt.has_value()) {
                response.set_status_code(status_codes::Unauthorized);
                response.set_body(json::value::object({ {U("error"), json::value::string(U("No autorizado"))} }));
                return response;
            }

            DecodedUser user = userOpt.value();
                response = paypalController.createPayment(request, user.id);
            }
            // POST /paypal/{orderId}/capture
            
            else if (segments[2] == U("capture"))
            {
             auto userOpt = AuthMiddleware::authenticateRequest(request);
            if (!userOpt.has_value()) {
                response.set_status_code(status_codes::Unauthorized);
                response.set_body(json::value::object({ {U("error"), json::value::string(U("No autorizado"))} }));
                return response;
            }

            DecodedUser user = userOpt.value();
                response = paypalController.capturePayment(request, user.id);
            }
            else
            {
                response = http_response(status_codes::NotFound);
            }
        }

        //CARRIERS
        if (method == web::http::methods::GET && path == U("/carriers"))
        {
            CarrierController model;
            response = model.getCarriers(request);
        }

        //CATEGORIES

        if (method == web::http::methods::GET && path == U("/categories"))
        {
            CategoryController categoryController;
            response = categoryController.getAllCategories();
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
