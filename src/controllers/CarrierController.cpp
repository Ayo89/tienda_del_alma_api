#include "controllers/CarrierController.h"

CarrierController::CarrierController() {}

web::http::http_response CarrierController::getCarriers(const web::http::http_request &request)
{
    web::http::http_response response;

    // get carriers
    CarrierModel carrierModel;
    auto [optCarriers, errors] = carrierModel.getAllCarriers();
    if (!optCarriers.has_value())
    {
        response.set_status_code(web::http::status_codes::NotFound);
        response.set_body(U("No carriers found"));
        return response;
    }

    response.set_status_code(web::http::status_codes::OK);

    web::json::value json_response = web::json::value::array();
    for (size_t i = 0; i < optCarriers->size(); ++i)
    {
        const auto &carrier = optCarriers->at(i);
        json_response[i] = web::json::value::object();
        json_response[i][U("id")] = web::json::value::number(carrier.id);
        json_response[i][U("name")] = web::json::value::string(carrier.name);
        json_response[i][U("price")] = web::json::value::number(carrier.price);
    }
    response.set_body(json_response);
    return response;
}

/* web::http::http_response getCarrierById(const web::http::http_request &request)
{
    web::http::http_response response;

    std::optional<std::string> optUserId = AuthUtils::getUserFromRequest(request);
    if (!optUserId.has_value()) // If token is not provided or is invalid
    {
        response.set_status_code(web::http::status_codes::Unauthorized);
        response.set_body(U("Token not provided or invalid"));
        return response;
    }
    int user_id = std::stoi(optUserId.value());

    const optOrder = orderModel.getOrderById(order_id, user_id);
    if (!optOrder.has_value())
    {
        response.set_status_code(web::http::status_codes::NotFound);
        response.set_body(U("Order not found"));
        return response;
    }

    CarrierModel carrierModel;
    auto [optCarrier, errors] = carrierModel.getCarrierById(carrier_id);
    if (!optCarrier.has_value())
    {
        response.set_status_code(web::http::status_codes::NotFound);
        response.set_body(U("No carriers found"));
        return response;
    }

    response.set_status_code(web::http::status_codes::OK);

    const Carrier &carrier = optCarrier.value();

    web::json::value json_response = web::json::value::object();
    json_response[U("id")] = web::json::value::number(carrier.id);
    json_response[U("name")] = web::json::value::string(carrier.name);
    json_response[U("price")] = web::json::value::number(carrier.price);

    response.set_status_code(web::http::status_codes::OK);
    response.set_body(json_response);
    return response;
} */