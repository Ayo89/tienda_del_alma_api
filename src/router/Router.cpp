
#include "router/Router.h"
#include "server/Server.h"

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
        else if (method == web::http::methods::GET && path == U("/users"))
        {
            
        }


        //PRODUCTS

        if(method == web::http::methods::GET && path == U("/products"))
        {
            ProductController model;
            response = model.getAllProducts();
        }

        // SHIPPING ADDRESS

      if(method == web::http::methods::GET && path == U("/address"))
        {
            
            response = addressController.getAddressesByUserId(request);
        } 
        else if (method == web::http::methods::GET && path.find(U("/address/")) != std::string::npos)
        {
            
            response = addressController.getAddressById(request);
        }
        else if (method == web::http::methods::POST && path == U("/address"))
        {
            
            response = addressController.createAddress(request);
        }
        else if(method == web::http::methods::PUT && path.find(U("/address")) != std::string::npos)
        {
            
            response = addressController.updateAddress(request);
        }
        else if(method == web::http::methods::DEL && path.find(U("/address/")) != std::string::npos)
        {
            
            response = addressController.deleteAddress(request);
        }

        // ORDERS

        if (method == web::http::methods::POST && path == U("/order"))
        {
            
            response = orderController.createOrder(request);

        } 
        else if (method == web::http::methods::GET && path == U("/order"))
        {
            response = orderController.getOrdersByUserId(request);
        } 
        else if(method == web::http::methods::GET && path.find(U("/order/")) != std::string::npos)
        {
            response = orderController.getOrderById(request);
        }
        else if (method == web::http::methods::PUT)
        {
            auto path_segments = web::uri::split_path(request.request_uri().path());

            if (path_segments.size() == 4 &&
                path_segments[0] == U("order") &&
                path_segments[2] == U("carrier"))
            {
                response = orderController.updateTotalbyOrderId(request);
            }
        }

        //PAYPAL

        if (method == web::http::methods::POST && path.find(U("/paypal/")) == 0)
        {
            PaypalController paypalController;
            response = paypalController.createPayment(request);
        }

        //CARRIERS
        if (method == web::http::methods::GET && path == U("/carriers"))
        {
            CarrierController model;
            response = model.getCarriers(request);
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
