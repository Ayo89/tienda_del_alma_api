#ifndef ADDRESSCONTROLLER_H
#define ADDRESSCONTROLLER_H

#include <cpprest/http_msg.h>
#include <cpprest/json.h>
#include <optional>
#include <string>
#include "db/DatabaseConnection.h"
#include "entities/Address.h"
#include "model/AddressModel.h"
#include "AuthUtils.h"
#include "services/jwt/JwtService.h"

class AddressController
{
private:
    AddressModel model;

public:
    AddressController();
    web::http::http_response createAddress(const web::http::http_request &request, const int userId);
    web::http::http_response getAddressesByUserId(const web::http::http_request &request, const int user_id);
    web::http::http_response getAddressById(const web::http::http_request &request, const int user_id);
    web::http::http_response updateAddress(const web::http::http_request &request, const int user_id);
    web::http::http_response deleteAddress(const web::http::http_request &request, const int user_id);
    web::http::http_response setDefaultAddressController(const web::http::http_request &request, const int user_id);
};

#endif