#ifndef CARRIERCONTROLLER_H
#define CARRIERCONTROLLER_H

#include <cpprest/http_msg.h>
#include <cpprest/json.h>
#include <optional>
#include <string>
#include "db/DatabaseConnection.h"
#include "entities/Carrier.h"
#include "model/CarrierModel.h"
#include "AuthUtils.h"
#include "services/jwt/JwtService.h"
#include "utils/Errors.h"

class CarrierController
{

public:
    CarrierController(/* args */);
    web::http::http_response getCarriers(const web::http::http_request &request);
    web::http::http_response getCarrierById(const web::http::http_request &request);
};

#endif
