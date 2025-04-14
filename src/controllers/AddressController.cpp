#include "controllers/AddressController.h"
#include "services/jwt/JwtService.h"

web::http::http_response AddressController::createAddress(const web::http::http_request &request)
{
    // 1. Verificar si el token es válido y obtener el user_id
    std::optional<std::string> user_id = AuthUtils::getUserIdFromRequest(request);
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

    std::string type = json_data.has_field(U("type")) ? json_data[U("type")].as_string() : "";
    bool is_default = json_data.has_field(U("is_default")) ? json_data[U("is_default")].as_bool() : false;
    std::string additional_info = json_data.has_field(U("additional_info")) ? json_data[U("additional_info")].as_string() : "";
    // 5. Crear la dirección utilizando el AddressModel (sin necesidad de pasar db)
    std::optional<int> address_id = model.createAddress(std::stoi(user_id.value()), first_name, last_name, phone, street, city, province, postal_code, country, type, is_default, additional_info);

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

web::http::http_response AddressController::getAddressesByUserId(const web::http::http_request &request)
{
    web::http::http_response response;

    // Obtener user_id a través del token (desde cookies)
    std::optional<std::string> user_id = AuthUtils::getUserIdFromRequest(request);
    if (!user_id.has_value())
    {
        response.set_status_code(web::http::status_codes::Unauthorized);
        response.set_body(U("Token no proporcionado o inválido"));
        return response;
    }

    // Consultar direcciones del usuario
    auto addresses = model.getAllAddressByUserId(std::stoi(user_id.value()));

    response.set_status_code(web::http::status_codes::OK);
    if (addresses.has_value() && !addresses->empty())
    {
        web::json::value json_response = web::json::value::array();
        for (size_t i = 0; i < addresses->size(); ++i)
        {
            const auto &address = addresses->at(i);
            web::json::value json_address;
            json_address[U("id")] = web::json::value::number(address.id);
            json_address[U("first_name")] = web::json::value::string(address.first_name);
            json_address[U("last_name")] = web::json::value::string(address.last_name);
            json_address[U("phone")] = web::json::value::string(address.phone);
            json_address[U("street")] = web::json::value::string(address.street);
            json_address[U("city")] = web::json::value::string(address.city);
            json_address[U("province")] = web::json::value::string(address.province);
            json_address[U("postal_code")] = web::json::value::string(address.postal_code);
            json_address[U("country")] = web::json::value::string(address.country);
            json_address[U("is_default")] = web::json::value::boolean(address.is_default);
            json_address[U("additional_info")] = web::json::value::string(address.additional_info);
            json_address[U("type")] = web::json::value::string(address.type);
            json_response[i] = json_address;
        }
        response.set_body(json_response);
    }
    else
    {
        web::json::value empty_msg = web::json::value::object();
        empty_msg[U("message")] = web::json::value::string(U("No se encontraron direcciones para el usuario"));
        response.set_body(empty_msg);
    }

    return response;
}