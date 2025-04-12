#include "controllers/AddressController.h"
#include "services/jwt/JwtService.h"

web::http::http_response AddressController::createAddress(const web::http::http_request &request)
{
    // 1. Extraer el token del encabezado 'Authorization'
    auto cookies_header = request.headers().find(U("Cookie"));
    std::string token = "";

    if (cookies_header != request.headers().end())
    {
        std::string cookies = utility::conversions::to_utf8string(cookies_header->second);

        // Buscar el token en las cookies, por ejemplo 'token=<jwt_token>'
        size_t token_start = cookies.find("token=");
        if (token_start != std::string::npos)
        {
            token_start += 6; // Skip "token="
            size_t token_end = cookies.find(";", token_start);
            if (token_end == std::string::npos)
            {
                token_end = cookies.length(); // Si no hay más cookies, tomar hasta el final
            }
            token = cookies.substr(token_start, token_end - token_start);
        }
    }

    // 2. Si no encontramos el token en las cookies
    if (token.empty())
    {
        web::http::http_response response(web::http::status_codes::Unauthorized);
        response.set_body(U("Token no proporcionado"));
        return response;
    }
    // 3. Decodificar el token para obtener el user_id
    std::optional<std::string> user_id = JwtService::decodeToken(token);
    if (!user_id.has_value())
    {
        // Si el token es inválido o ha expirado
        web::http::http_response response(web::http::status_codes::Unauthorized);
        response.set_body(U("Token inválido o expirado"));
        return response; // Devuelves la respuesta aquí
    }

    // 4. Obtener los datos del cuerpo de la solicitud (como la dirección)
    web::json::value json_data = request.extract_json().get();
    std::string first_name = json_data[U("first_name")].as_string();
    std::string last_name = json_data[U("last_name")].as_string();
    std::string phone = json_data[U("phone")].as_string();
    std::string street = json_data[U("street")].as_string();
    std::string city = json_data[U("city")].as_string();
    std::string province = json_data[U("province")].as_string();
    std::string postal_code = json_data[U("postal_code")].as_string();
    std::string country = json_data[U("country")].as_string();

    bool is_default = json_data.has_field(U("is_default")) ? json_data[U("is_default")].as_bool() : false;
    std::string additional_info = json_data.has_field(U("additional_info")) ? json_data[U("additional_info")].as_string() : "";
    // 5. Crear la dirección utilizando el AddressModel (sin necesidad de pasar db)
    std::optional<int> address_id = model.createAddress(std::stoi(user_id.value()), first_name, last_name, phone, street, city, province, postal_code, country, is_default, additional_info);

    if (address_id.has_value())
    {
        web::http::http_response response(web::http::status_codes::Created);
        response.set_body(U("Dirección creada exitosamente"));
        return response; // Devuelves la respuesta aquí
    }
    else
    {
        web::http::http_response response(web::http::status_codes::InternalError);
        response.set_body(U("Error al crear la dirección"));
        return response; // Devuelves la respuesta aquí
    }
}