#include "services/PaypalService.h"
#include "env/EnvLoader.h"
// Constructor
PaypalService::PaypalService()
{
}
http_response PaypalService::createPayment(const std::string &total, const std::string &idempotencyKey)
{

    std::string token = getAccessToken();
    if (token.empty())
    {
        std::cerr << "Error al obtener el token de acceso" << std::endl;
        return http_response(status_codes::InternalError);
    }

    // Generar una clave de identificación unica

    // Crear encabezados
    http_request request(methods::POST);
    request.headers().add(U("Content-Type"), U("application/json"));
    request.headers().add(U("Authorization"), U("Bearer ") + utility::conversions::to_string_t(token));
    request.headers().add(U("Accept"), U("application/json"));
    request.headers().add(U("Accept-Language"), U("en_US"));
    request.headers().add(U("PayPal-Request-Id"),
                          utility::conversions::to_string_t(idempotencyKey));
    request.headers().add(U("Prefer"), U("return=representation"));

    // Crear el cuerpo de la solicitud
    std::string apiBase = "https://api-m.sandbox.paypal.com/v2/checkout/orders";
    web::json::value body = web::json::value::object();
    body[U("intent")] = web::json::value::string(U("CAPTURE"));

    // Configuración moderna en payment_source.paypal.experience_context
    web::json::value paymentSource = web::json::value::object();
    web::json::value paypalContext = web::json::value::object();
    web::json::value experience = web::json::value::object();
    experience[U("shipping_preference")] = web::json::value::string(U("NO_SHIPPING"));
    experience[U("user_action")] = web::json::value::string(U("PAY_NOW"));
    paypalContext[U("experience_context")] = experience;
    paymentSource[U("paypal")] = paypalContext;
    body[U("payment_source")] = paymentSource;

    web::json::value amount = web::json::value::object();
    amount[U("currency_code")] = web::json::value::string(U("EUR"));
    amount[U("value")] = web::json::value::string(total);

    web::json::value purchase_unit = web::json::value::object();
    purchase_unit[U("amount")] = amount;

    web::json::value purchase_units = web::json::value::array();
    purchase_units[0] = purchase_unit;

    body[U("purchase_units")] = purchase_units;

    request.set_body(body);
    // Enviar petición y obtener respuesta
    http_response response = web::http::client::http_client(apiBase)
                                 .request(request)
                                 .get();
    // Comprobar el código de estado de la respuesta
    if (response.status_code() == status_codes::Created || response.status_code() == status_codes::OK)
    {
        {
            auto json = response.extract_json().get();
            std::string order_id = utility::conversions::to_utf8string(json[U("id")].as_string());
            web::json::value result = web::json::value::object();
            result[U("orderID")] = web::json::value::string(utility::conversions::to_string_t(order_id));
            result[U("idempotency_key")] = web::json::value::string(idempotencyKey);
            result[U("status")] = json[U("status")];
            http_response customResponse(status_codes::OK);
            customResponse.set_body(result);
            return customResponse;
        }
    }
    else if (response.status_code() == status_codes::Unauthorized)
    {
        return http_response(status_codes::Unauthorized);
    }
    else if (response.status_code() == status_codes::BadRequest)
    {
        return http_response(status_codes::BadRequest);
    }
    else
    {
        std::cerr << "Error inesperado: " << response.status_code() << std::endl;
        return http_response(status_codes::InternalError);
    }
}

std::string PaypalService::getAccessToken()
{
    std::string apiBaseToken = "https://api-m.sandbox.paypal.com/v1/oauth2/token";

    // Crear encabezados
    http_request request(methods::POST);
    request.headers().add(U("Content-Type"), U("application/x-www-form-urlencoded"));
    request.headers().add(U("Accept"), U("application/json"));
    request.headers().add(U("Accept-Language"), U("en_US"));

    EnvLoader env(".env");
    env.load();

    std::string clientId = env.get("PAYPAL_CLIENT_ID", "");
    std::string clientSecret = env.get("PAYPAL_CLIENT_SECRET", "");

    // Autenticación Basic con clientId:clientSecret en base64
    std::string encodedCredentials = UtilsOwner::base64_encode(clientId + ":" + clientSecret);
    request.headers().add(U("Authorization"), U("Basic ") + encodedCredentials);

    // Cuerpo de la solicitud
    request.set_body(U("grant_type=client_credentials"));

    // Enviar petición y obtener respuesta
    http_response response = web::http::client::http_client(apiBaseToken)
                                 .request(request)
                                 .get();
    // Comprobar el código de estado de la respuesta
    if (response.status_code() == status_codes::OK)
    {
        auto jsonResponse = response.extract_json().get();
        return utility::conversions::to_utf8string(jsonResponse[U("access_token")].as_string());
    }
    else if (response.status_code() == status_codes::Unauthorized)
    {
        std::cerr << "Error de autenticación: " << response.status_code() << std::endl;
        return "";
    }
    else if (response.status_code() == status_codes::BadRequest)
    {
        std::cerr << "Error en la solicitud: " << response.status_code() << std::endl;
        return "";
    }
    else
    {
        std::cerr << "Error inesperado: " << response.status_code() << std::endl;
        return "";
    }
}

http_response PaypalService::capturePayment(const std::string &orderID)
{
    std::cout << "Capturando pago..." << std::endl;
    std::string apiBase = "https://api-m.sandbox.paypal.com/v2/checkout/orders/" + orderID + "/capture";
    http_request request(methods::POST);
    request.headers().add(U("Content-Type"), U("application/json"));
    request.headers().add(U("Accept"), U("application/json"));
    request.headers().add(U("PayPal-Request-Id"), U("unique-request-id"));
    request.headers().add(U("Prefer"), U("return=representation"));

    // Obtener el token de acceso
    std::string token = getAccessToken();
    if (token.empty())
    {
        std::cerr << "Error al obtener el token de acceso" << std::endl;
        return http_response(status_codes::InternalError);
    }
    request.headers().add(U("Authorization"), U("Bearer ") + utility::conversions::to_string_t(token));

    // Enviar petición y obtener respuesta
    http_response response = web::http::client::http_client(apiBase)
                                 .request(request)
                                 .get();

    // Comprobar el código de estado de la respuesta
    if (response.status_code() == status_codes::Created || response.status_code() == status_codes::OK)
    {
        return response;
    }
    else if (response.status_code() == status_codes::Unauthorized)
    {
        return http_response(status_codes::Unauthorized);
    }
    else if (response.status_code() == status_codes::BadRequest)
    {
        return http_response(status_codes::BadRequest);
    }
    else
    {
        std::cerr << "Error inesperado: " << response.status_code() << std::endl;
        return http_response(status_codes::InternalError);
    }
}