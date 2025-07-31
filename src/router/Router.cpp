
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
    // Manejo de preflight (CORS)
    listener_.support(web::http::methods::OPTIONS, [](const web::http::http_request &request)
                      {
        web::http::http_response response(status_codes::OK);
        Server::add_cors_headers(response);
        request.reply(response); });

    // Manejo general en hilo separado
    listener_.support([this](const web::http::http_request &request)
                      { pplx::create_task([=]()
                                          {
            try {
                auto path = request.relative_uri().path();
                auto method = request.method();
                web::http::http_response response(status_codes::NotFound);
                response.set_body(U("Ruta no encontrada"));

                // USERS
                if (method == web::http::methods::POST && path == U("/signup"))
                    response = authController.signup(request);
                else if (method == web::http::methods::POST && path == U("/login"))
                    response = authController.login(request);
                else if (method == web::http::methods::POST && path == U("/auth-google"))
                    response = authController.googleLogin(request);

                // PRODUCTS
                else if (method == web::http::methods::GET && path == U("/products")) {
                    ProductController model;
                    response = model.getAllProducts();
                }

                // SHIPPING ADDRESS
                auto segments_addresses = web::uri::split_path(request.request_uri().path());
                if (path.find(U("/address")) != std::string::npos) {
                    auto userOpt = AuthMiddleware::authenticateRequest(request);
                    if (!userOpt.has_value()) {
                        response.set_status_code(status_codes::Unauthorized);
                        response.set_body(json::value::object({ {U("error"), json::value::string(U("No autorizado"))} }));
                    } else {
                        DecodedUser user = userOpt.value();
                        if (method == web::http::methods::GET && path == U("/address"))
                            response = addressController.getAddressesByUserId(request, user.id);
                        else if (method == web::http::methods::GET)
                            response = addressController.getAddressById(request, user.id);
                        else if (method == web::http::methods::POST)
                            response = addressController.createAddress(request, user.id);
                        else if (method == web::http::methods::PUT && path.find(U("default")) != std::string::npos)
                            response = addressController.setDefaultAddressController(request, user.id);
                        else if (method == web::http::methods::PUT)
                            response = addressController.updateAddress(request, user.id);
                        else if (method == web::http::methods::DEL)
                            response = addressController.deleteAddress(request, user.id);
                    }
                }

                // ORDERS
                else if (path.find(U("/order")) != std::string::npos) {
                    auto userOpt = AuthMiddleware::authenticateRequest(request);
                    if (!userOpt.has_value()) {
                        response.set_status_code(status_codes::Unauthorized);
                        response.set_body(json::value::object({ {U("error"), json::value::string(U("No autorizado"))} }));
                    } else {
                        DecodedUser user = userOpt.value();
                        if (method == web::http::methods::POST && path == U("/order")){
                            std::cout << "Creating order in router user_id: " << user.id << std::endl;
                                response = orderController.createOrder(request, user.id);
                        }
                        else if (method == web::http::methods::GET && path == U("/order"))
                            response = orderController.getOrdersByUserId(request, user.id);
                        else if (method == web::http::methods::GET)
                            response = orderController.getOrderById(request, user.id);
                        else if (method == web::http::methods::PUT) {
                            auto path_segments = web::uri::split_path(request.request_uri().path());
                            if (path_segments.size() == 4 &&
                                path_segments[0] == U("order") &&
                                path_segments[2] == U("carrier")) {
                                response = orderController.updateTotalbyOrderId(request, user.id);
                            }
                        }
                    }
                }

                // PAYPAL
                auto segments = web::uri::split_path(request.request_uri().path());
                if ((method == methods::POST) &&
                    ((segments.size() == 3 || segments.size() == 4) && segments[0] == U("paypal"))) {
                    auto userOpt = AuthMiddleware::authenticateRequest(request);
                    if (!userOpt.has_value()) {
                        response.set_status_code(status_codes::Unauthorized);
                        response.set_body(json::value::object({ {U("error"), json::value::string(U("No autorizado"))} }));
                    } else {
                        DecodedUser user = userOpt.value();
                        PaypalController paypalController;
                        if (segments[2] == U("create"))
                            response = paypalController.createPayment(request, user.id);
                        else if (segments[2] == U("capture"))
                            response = paypalController.capturePayment(request, user.id);
                        else
                            response = http_response(status_codes::NotFound);
                    }
                }

                // CARRIERS
                else if (method == web::http::methods::GET && path == U("/carriers")) {
                    CarrierController model;
                    response = model.getCarriers(request);
                }

                // CATEGORIES
                else if (method == web::http::methods::GET && path == U("/categories")) {
                    CategoryController categoryController;
                    response = categoryController.getAllCategories();
                }

                // Añadir headers CORS SIEMPRE
                Server::add_cors_headers(response);

                // Añadir cookie si hay token
                if (response.headers().has(U("X-Token"))) {
                    auto token = response.headers()[U("X-Token")];
                    Server::add_cookie(response, utility::conversions::to_utf8string(token));
                    response.headers().remove(U("X-Token"));
                }

                request.reply(response);
            } catch (const std::exception &e) {
                std::cerr << "[Router Error] Excepción al procesar la request: " << e.what() << std::endl;
                web::http::http_response errorResponse(status_codes::InternalError);
                Server::add_cors_headers(errorResponse);
                errorResponse.set_body(U("Error interno del servidor"));
                request.reply(errorResponse);
            } }); });
}
