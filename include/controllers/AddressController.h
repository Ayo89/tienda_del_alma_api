#ifndef ADDRESSCONTROLLER_H
#define ADDRESSCONTROLLER_H
#include <cpprest/http_msg.h>
#include <cpprest/http_msg.h>
#include <cpprest/json.h>
#include <optional>
#include <string>
#include "db/DatabaseConnection.h"
#include "entities/Address.h"
#include "model/AddressModel.h"
#include "AuthUtils.h"

class AddressController
{
private:
    AddressModel model;

public:
    AddressController(DatabaseConnection &db) : model(db) {} // Pasamos db al constructor de UserModel
    web::http::http_response createAddress(const web::http::http_request &request);
    web::http::http_response getAddressesByUserId(const web::http::http_request &request);
    web::http::http_response getAddressById(const web::http::http_request &request);
};

#endif