#include "controllers/AddressController.h"

AddressController::AddressController() {}

web::http::http_response AddressController::createAddress(const web::http::http_request &request)
{
    // 1. Verifying user_id from token
    std::optional<std::string> user_id = AuthUtils::getUserIdFromRequest(request);
    if (!user_id.has_value())
    {
        // Si el token es inválido o ha expirado
        web::http::http_response response(web::http::status_codes::Unauthorized);
        response.set_body(U("Unauthorized: Invalid Token or Expired"));
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
        response.set_body(U("Address created successfully"));
        return response; // Devuelves la respuesta aquí
    }
    else
    {
        web::http::http_response response(web::http::status_codes::InternalError);
        response.set_body(U("Failed to create address"));
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
        response.set_body(U("Invalid Token or Expired"));
        return response;
    }
    std::cout << "User ID: " << user_id.value() << std::endl;
    // Consultar direcciones del usuario
    auto addresses = model.getAllAddressByUserId(std::stoi(user_id.value()));

    if (!addresses.has_value())
    {
        std::cout << "⚠️ Addresses not has value" << std::endl;
    }
    else if (addresses->empty())
    {
        std::cout << "✅ No addresses found" << std::endl;
    }
    else
    {
        std::cout << "✅ Addresses found: " << addresses->size() << std::endl;
    }

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
            json_address[U("created_at")] = web::json::value::string(address.created_at);
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

web::http::http_response AddressController::getAddressById(const web::http::http_request &request)
{
    web::http::http_response response;
    std::optional<std::string> user_id = AuthUtils::getUserIdFromRequest(request);
    if (!user_id.has_value())
    {
        response.set_status_code(web::http::status_codes::Unauthorized);
        response.set_body(U("Token no proporcionado o inválido"));
        return response;
    }
    // Obtain address_id from URL
    auto path = request.request_uri().path();
    auto address_id_str = path.substr(path.find_last_of('/') + 1);
    int address_id = std::stoi(address_id_str);

    // Obtain type from query parameters
    auto query = web::uri::split_query(request.request_uri().query());
    std::string type = "shipping"; // valor por defecto

    auto it = query.find("type");
    if (it != query.end())
    {
        type = it->second;
    }

    // Check if the address_id is valid
    auto address = model.getAddressById(address_id, std::stoi(user_id.value()), type);
    if (!address.has_value())
    {
        response.set_status_code(web::http::status_codes::NotFound);
        response.set_body(U("Address not found"));
        return response;
    }
    else
    {
        std::cout << "✅ Address found: " << address->id << std::endl;
        web::json::value json_response;
        json_response[U("id")] = web::json::value::number(address->id);
        json_response[U("first_name")] = web::json::value::string(address->first_name);
        json_response[U("last_name")] = web::json::value::string(address->last_name);
        json_response[U("phone")] = web::json::value::string(address->phone);
        json_response[U("street")] = web::json::value::string(address->street);
        json_response[U("city")] = web::json::value::string(address->city);
        json_response[U("province")] = web::json::value::string(address->province);
        json_response[U("postal_code")] = web::json::value::string(address->postal_code);
        json_response[U("country")] = web::json::value::string(address->country);
        json_response[U("is_default")] = web::json::value::boolean(address->is_default);
        json_response[U("additional_info")] = web::json::value::string(address->additional_info);
        json_response[U("created_at")] = web::json::value::string(address->created_at);
        json_response[U("type")] = web::json::value::string(address->type);
        response.set_body(json_response);
        response.set_status_code(web::http::status_codes::OK);
        return response;
    };
}

// update
web::http::http_response AddressController::updateAddress(const web::http::http_request &request)
{
    web::http::http_response response;

    // 1. Obtainer user_id from token (from cookies)
    std::optional<std::string> user_id = AuthUtils::getUserIdFromRequest(request);
    if (!user_id.has_value())
    {
        response.set_status_code(web::http::status_codes::Unauthorized);
        response.set_body(U("Invalid Token or Expired"));
        return response;
    }

    // 2. Obtener address_id from URL
    auto path = request.request_uri().path();
    auto address_id_str = path.substr(path.find_last_of('/') + 1);
    int address_id = std::stoi(address_id_str);

    // 3. Obtainer type from URL
    std::string type = path.find("/billing/") != std::string::npos ? "billing" : "shipping";

    // 4. Obtain JSON body from request
    web::json::value body;
    try
    {
        body = request.extract_json().get();
    }
    catch (const std::exception &e)
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Cuerpo JSON inválido"));
        return response;
    }

    // 5. Extract fields from JSON body
    try
    {
        std::string first_name = utility::conversions::to_utf8string(body[U("first_name")].as_string());
        std::string last_name = utility::conversions::to_utf8string(body[U("last_name")].as_string());
        std::string phone = utility::conversions::to_utf8string(body[U("phone")].as_string());
        std::string street = utility::conversions::to_utf8string(body[U("street")].as_string());
        std::string city = utility::conversions::to_utf8string(body[U("city")].as_string());
        std::string province = utility::conversions::to_utf8string(body[U("province")].as_string());
        std::string postal_code = utility::conversions::to_utf8string(body[U("postal_code")].as_string());
        std::string country = utility::conversions::to_utf8string(body[U("country")].as_string());
        std::string additional_info = utility::conversions::to_utf8string(body[U("additional_info")].as_string());

        // 6. Execute updateAddress method
        auto [result_updateAddress, error_updateAddress] = model.updateAddress(
            std::stoi(user_id.value()),
            address_id,
            first_name,
            last_name,
            phone,
            street,
            city,
            province,
            postal_code,
            country,
            type,
            additional_info);

        // 7. Verify if the address was updated successfully
        if (result_updateAddress.has_value())
        {
            if (result_updateAddress.value())
            {
                std::cout << "✅ Address updated successfully" << std::endl;
            }
            else if (error_updateAddress == Errors::NoRowsAffected)
            {
                std::cout << "⚠️ No rows affected" << std::endl;
            }
            else
            {
                std::cout << "❌ Address not updated" << std::endl;
                response.set_status_code(web::http::status_codes::InternalError);
                response.set_body(U("Failed to update address"));
                return response;
            }
        }

        // 8. Responder con éxito
        response.set_status_code(web::http::status_codes::OK);
        response.set_body(U("Address updated successfully"));
        return response;
    }
    catch (const std::exception &e)
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Invalid JSON body"));
        return response;
    }
}

// delete
web::http::http_response AddressController::deleteAddress(const web::http::http_request &request)
{
    web::http::http_response response;

    // 1. Obtainer user_id from token (from cookies)
    std::optional<std::string> user_id = AuthUtils::getUserIdFromRequest(request);
    if (!user_id.has_value())
    {
        response.set_status_code(web::http::status_codes::Unauthorized);
        response.set_body(U("Invalid Token or Expired"));
        return response;
    }
    // 2. Obtainer address_id from URL
    auto path = request.request_uri().path();
    auto address_id_str = path.substr(path.find_last_of('/') + 1);
    int address_id = std::stoi(address_id_str);
    int user_id_int = std::stoi(user_id.value());
    auto query = web::uri::split_query(request.request_uri().query());
    std::string type = "shipping"; // valor por defecto

    auto it = query.find("type");
    if (it != query.end())
    {
        type = it->second;
    }
    auto allAddressesOpt = model.getAllAddressByUserId(user_id_int);

    if (!allAddressesOpt.has_value())
    {

        response.set_status_code(web::http::status_codes::InternalError);
        response.set_body(U("Failed to retrieve addresses"));
        return response;
    }

    std::vector<Address> addresses = allAddressesOpt.value();

    int billingCount = 0;
    int shippingCount = 0;

    for (const auto &addr : addresses)
    {
        if (addr.type == "billing")
        {
            billingCount++;
        }
        else if (addr.type == "shipping")
        {
            shippingCount++;
        }
    }
    // 3. Check if the address is the last one of its type
    if (billingCount <= 1 && type == "billing" || shippingCount <= 1 && type == "shipping")
    {
        response.set_status_code(web::http::status_codes::InternalError);
        response.set_body(U("❌ You Can't Delete The Last Address"));
        return response;
    }
    // 4. Execute deleteAddress method
    auto result = model.deleteAddress(user_id_int, address_id, type);

    // 5. Verify if the address was deleted successfully
    if (!result.has_value())
    {
        response.set_status_code(web::http::status_codes::InternalError);
        response.set_body(U("Failed to delete address"));
        return response;
    }

    // 6. Responder con éxito
    response.set_status_code(web::http::status_codes::OK);
    response.set_body(U("Address deleted successfully"));
    return response;
}

web::http::http_response AddressController::setDefaultAddressController(const web::http::http_request &request)
{
    web::http::http_response response;

    // 1. Obtainer user_id from token (from cookies)
    std::optional<std::string> user_id = AuthUtils::getUserIdFromRequest(request);
    if (!user_id.has_value())
    {
        response.set_status_code(web::http::status_codes::Unauthorized);
        response.set_body(U("Invalid Token or Expired"));
        return response;
    }
    // 2. Obtainer address_id from params
    auto path = web::uri::split_path(request.request_uri().path());
    if (path.size() < 4)
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Invalid URL"));
        return response;
    }

    std::string type = path[2];
    int address_id = std::stoi(path[3]);

    // 4. Execute setDefaultAddress method
    auto [result, error] = model.setDefaultAddress(std::stoi(user_id.value()), address_id, type);

    // 5. Verify if the address was set as default successfully
    if (result.has_value() && result.value() || error == Errors::NoError)
    {
        std::cout << "✅ Address set as default successfully" << std::endl;
        response.set_status_code(web::http::status_codes::OK);
        response.set_body(U("Address set as default successfully"));
        return response;
    }
    else if (error == Errors::NoRowsAffected)
    {
        std::cout << "⚠️ No rows affected" << std::endl;
        response.set_status_code(web::http::status_codes::OK);
        response.set_body(U("no rows affected"));
        return response;
    }
    else
    {
        std::cout << "❌ Failed to set address as default" << std::endl;
        response.set_status_code(web::http::status_codes::InternalError);
        response.set_body(U("Failed to set address as default"));
        return response;
    }
}