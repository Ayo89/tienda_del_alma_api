#include "controllers/PaypalController.h"

PaypalController::PaypalController() {}
OrderModel orderModel;

web::http::http_response PaypalController::createPayment(const web::http::http_request &request)
{
    web::http::http_response response;

    std::optional<std::string> optUserId = AuthUtils::getUserIdFromRequest(request);

    if (!optUserId.has_value()) // If token is not provided or is invalid
    {
        response.set_status_code(web::http::status_codes::Unauthorized);
        response.set_body(U("Token not provided or invalid"));
        return response;
    }
    int user_id = std::stoi(optUserId.value());

    auto path = request.request_uri().path();
    auto order_id_str = path.substr(path.find_last_of('/') + 1);
    if (order_id_str.empty())
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Invalid order ID"));
        return response;
    }
    int order_id = std::stoi(order_id_str);

    auto optOrder = orderModel.getOrderById(order_id, user_id);

    if (!optOrder.has_value())
    {
        response.set_status_code(web::http::status_codes::NotFound);
        response.set_body(U("Order not found"));
        return response;
    }
    Order order = optOrder.value();

    

    if (order.status == "COMPLETED")
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Order already paid"));
        return response;
    }
    if (order.status == "CANCELLED")
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Order cancelled"));
        return response;
    }

    PaypalService paypalService;
    const auto total = UtilsOwner::toString2Dec(order.total);
    if (total.empty())
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Invalid total amount"));
        return response;
    }
    try
    {
        OrderItemModel orderItemModel;
        // items in the cart
        auto [items, errors] = orderItemModel.getOrderItemsByOrderId(order_id);

        if (errors != Errors::NoError){
            response.set_status_code(web::http::status_codes::NotFound);
            response.set_body(U("Failed to get order items"));
            return response;
        }
       
        std::string current_cart_hash = items.has_value() ? UtilsOwner::hashCart(items.value()) : "";
        
        auto paymentResponse = paypalService.createPayment(total);
        auto jsonResponse = paymentResponse.extract_json().get();

        std::string storedPaypalId = order.paypal_order_id;
        std::string newPaypalId = utility::conversions::to_utf8string(
            jsonResponse[U("orderID")].as_string());
        std::string idempotency_key = jsonResponse[U("idempotency_key")].as_string();
        std::string status = jsonResponse[U("status")].as_string();
        std::cout << "idempotency_key: " << idempotency_key << std::endl;
        std::cout << "status: " << status << std::endl;
        std::cout << "userId: " << user_id << std::endl;
        std::cout << "cart_hash: " << current_cart_hash << std::endl;
        std::cout << "total: " << order.total << std::endl;
        std::cout << "storedPaypalId: " << storedPaypalId << std::endl;
        std::cout << "newPaypalId: " << newPaypalId << std::endl;

        PaymentAttempModel paymentAttemptModel;
        auto [paymentAttempt, created] = paymentAttemptModel.createPaymentAttempt(
            user_id,
            order_id,
            current_cart_hash,
            order.total,
            idempotency_key,
            storedPaypalId.empty() ? newPaypalId : storedPaypalId,
            status);

        web::json::value body;
        body[U("status")] = web::json::value::string(U("success"));
        body[U("paypalResponse")] = jsonResponse;
        response.set_body(body);
        response.set_status_code(paymentResponse.status_code());

        auto orderID = jsonResponse[U("orderID")].as_string();
        if (storedPaypalId != newPaypalId)
        {
            orderModel.updateOrderPaypalId(user_id, order_id, newPaypalId);
        }
    }
    catch (const std::exception &e)
    {
        response.set_status_code(web::http::status_codes::InternalError);
        response.set_body(U("Error processing payment: ") + utility::conversions::to_string_t(e.what()));
    }
    return response;
}

web::http::http_response PaypalController::capturePayment(const web::http::http_request &request)
{
    web::http::http_response response;

    std::optional<std::string> optUserId = AuthUtils::getUserIdFromRequest(request);
    if (!optUserId.has_value()) // If token is not provided or is invalid
    {
        response.set_status_code(web::http::status_codes::Unauthorized);
        response.set_body(U("Token not provided or invalid"));
        return response;
    }
    int user_id = std::stoi(optUserId.value());

    auto path = request.request_uri().path();
    auto order_id_str = path.substr(path.find_last_of('/') + 1);
    if (order_id_str.empty())
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Invalid order ID"));
        return response;
    }
    int order_id = std::stoi(order_id_str);

    OrderModel model;
    auto optOrder = model.getOrderById(order_id, user_id);


    if (!optOrder.has_value())
    {
        response.set_status_code(web::http::status_codes::NotFound);
        response.set_body(U("Order not found"));
        return response;
    }

    PaypalService paypalService;
    try
    {
        auto captureResponse = paypalService.capturePayment(order_id_str);

        response.set_status_code(captureResponse.status_code());
        response.set_body(captureResponse.extract_json().get());
    }
    catch (const std::exception &e)
    {
        response.set_status_code(web::http::status_codes::InternalError);
        response.set_body(U("Error processing payment: ") + utility::conversions::to_string_t(e.what()));
    }

    return response;
}