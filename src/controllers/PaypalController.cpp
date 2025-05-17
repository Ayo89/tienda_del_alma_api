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

    auto segments = web::uri::split_path(request.request_uri().path());
    auto order_id_str = segments[1];
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

        if (errors != Errors::NoError)
        {
            response.set_status_code(web::http::status_codes::NotFound);
            response.set_body(U("Failed to get order items"));
            return response;
        }

        // check if the hash of the cart has changed
        PaymentAttempModel paymentAttemptModel;
        auto [paymentAttemptsOpt, paymentAttemptsError] = paymentAttemptModel.getPaymentAttemptsByOrderId(order_id);

        std::string current_cart_hash = items.has_value() ? UtilsOwner::hashCart(items.value()) : "";
        std::string idempotencyKey = UtilsOwner::generateUuid(); // Default to new
        bool shouldCreateNewAttempt = true;

        if (paymentAttemptsOpt.has_value())
        {
            const auto &attempts = paymentAttemptsOpt.value();

            // Filtra sólo los intentos con el mismo hash actual
            std::vector<std::reference_wrapper<const PaymentAttempt>> matching;
            matching.reserve(attempts.size());
            for (const auto &att : attempts)
            {
                if (att.cart_hash == current_cart_hash)
                {
                    matching.push_back(std::cref(att));
                }
            }

            if (!matching.empty())
            {
                // Elige el más reciente de los que coinciden
                const auto &lastMatching = matching.back().get();
                idempotencyKey = lastMatching.idempotency_key;
                shouldCreateNewAttempt = false;
            }
        }

        auto paymentResponse = paypalService.createPayment(total, idempotencyKey);
        auto jsonResponse = paymentResponse.extract_json().get();
        std::cout << "current_cart_hash: " << current_cart_hash << std::endl;
        std::string storedPaypalId = order.paypal_order_id;
        std::string newPaypalId = utility::conversions::to_utf8string(
            jsonResponse[U("orderID")].as_string());
        std::string status = jsonResponse[U("status")].as_string();

        if (shouldCreateNewAttempt)
        {

            auto [paymentAttempt, created] = paymentAttemptModel.createPaymentAttempt(
                user_id,
                order_id,
                current_cart_hash,
                order.total,
                idempotencyKey,
                storedPaypalId.empty() ? newPaypalId : storedPaypalId,
                status);
        }

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
    std::cout << "entrando en capture payment" << std::endl;
    web::http::http_response response;

    std::optional<std::string> optUserId = AuthUtils::getUserIdFromRequest(request);
    if (!optUserId.has_value()) // If token is not provided or is invalid
    {
        response.set_status_code(web::http::status_codes::Unauthorized);
        response.set_body(U("Token not provided or invalid"));
        return response;
    }
    int user_id = std::stoi(optUserId.value());

    auto segments = web::uri::split_path(request.request_uri().path());
    auto order_id_paypal = segments[1];
    auto order_id_segment = segments[3];

    if (order_id_paypal.empty() || order_id_segment.empty())
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Invalid order ID"));
        return response;
    }
    int order_id = std::stoi(order_id_segment);

    OrderModel model;
    auto optOrder = model.getOrderById(order_id, user_id);

    if (!optOrder.has_value())
    {
        response.set_status_code(web::http::status_codes::NotFound);
        response.set_body(U("Order not found"));
        return response;
    }

    PaypalService paypalService;
    PaymentAttempModel paymentAttemptModel;
    try
    {
        auto captureResponse = paypalService.capturePayment(order_id_paypal);
        if (captureResponse.status_code() == web::http::status_codes::Created || captureResponse.status_code() == web::http::status_codes::OK)
        {
            orderModel.updateOrderStatus(order_id, user_id, "COMPLETED");
            paymentAttemptModel.updatePaymentAttemptStatus(order_id_paypal, order_id, user_id, "COMPLETED");
        }
        response.set_status_code(captureResponse.status_code());
        response.set_body(captureResponse.extract_json().get());
        
    }
    catch (const std::exception &e)
    {
        response.set_status_code(web::http::status_codes::InternalError);
        response.set_body(U("Error processing payment: ") + utility::conversions::to_string_t(e.what()));
    }
    std::cout << response.to_string() << std::endl;
    return response;
}