#include "controllers/OrderController.h"

OrderController::OrderController() {}

web::http::http_response OrderController::createOrder(const web::http::http_request &request)
{
    web::http::http_response response;

    // 1. Obtain user_id from JWT token
    auto optUserId = AuthUtils::getUserIdFromRequest(request);

    if (!optUserId)
    {
        response.set_status_code(web::http::status_codes::Unauthorized);
        response.set_body(U("Unauthorized"));
        return response;
    }
    int user_id;
    try
    {
        user_id = std::stoi(optUserId.value());
    }
    catch (const std::exception &)
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Invalid user_id"));
        return response;
    }

    // 2. Obtain JSON body
    web::json::value body;
    try
    {
        body = request.extract_json().get();
    }
    catch (const std::exception &)
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Invalid JSON body"));
        return response;
    }

    // 3. Validate required fields
    if (!body.has_field(U("shipping_address_id")) ||
        !body.has_field(U("billing_address_id")) ||
        !body.has_field(U("total")) ||
        !body.has_field(U("products")))
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Missing required fields: shipping_address_id, billing_address_id, total o products"));
        return response;
    }

    int shipping_address_id = body[U("shipping_address_id")].as_integer();
    int billing_address_id = body[U("billing_address_id")].as_integer();
    std::string status = body.has_field(U("status")) ? body[U("status")].as_string() : "pending";
    double total = body[U("total")].as_double();

    // 4. Parse products
    std::vector<OrderProduct> products;
    try
    {
        auto arr = body[U("products")].as_array();
        for (const auto &item : arr)
        {
            OrderProduct prod;
            prod.product_id = item.at(U("id")).as_integer();
            prod.quantity = item.at(U("quantity")).as_integer();
            prod.price = item.at(U("price")).as_double();
            products.push_back(prod);
        }
    }
    catch (const std::exception &e)
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Invalid product data: ") + utility::conversions::to_string_t(e.what()));
        return response;
    }

    // 5. Optional fields
    auto getOpt = [&](const utility::string_t &key)
    {
        return body.has_field(key) ? body[key].as_string() : U("");
    };
    std::string shipment_date = getOpt(U("shipment_date"));
    std::string delivery_date = getOpt(U("delivery_date"));
    std::string carrier = getOpt(U("carrier"));
    std::string tracking_url = getOpt(U("tracking_url"));
    std::string tracking_number = getOpt(U("tracking_number"));
    std::string payment_method = getOpt(U("payment_method"));
    std::string payment_status = getOpt(U("payment_status"));

    // 6. Call the model to create the order
    OrderModel model;
    auto optOrderId = model.createOrder(
        user_id,
        shipping_address_id,
        billing_address_id,
        status,
        total,
        products,
        shipment_date,
        delivery_date,
        carrier,
        tracking_url,
        tracking_number,
        payment_method,
        payment_status);

    if (!optOrderId)
    {
        response.set_status_code(web::http::status_codes::InternalError);
        response.set_body(U("Failure creating order"));
        return response;
    }

    // 7. Answer with the order ID
    // 7.1. Set the response body
    // 7.2. Set the status code
    // 7.3. Set the content type
    web::json::value respBody;
    respBody[U("order_id")] = web::json::value::number(optOrderId.value());
    respBody[U("message")] = web::json::value::string(U("Order created successfully"));

    response.set_status_code(web::http::status_codes::Created);
    response.headers().add(U("Content-Type"), U("application/json"));
    response.set_body(respBody);
    return response;
}
