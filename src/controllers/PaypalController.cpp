#include "controllers/PaypalController.h"

PaypalController::PaypalController() {}
OrderModel orderModel;

web::http::http_response PaypalController::createPayment(const web::http::http_request &request, const int user_id)
{
    web::http::http_response response;

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

    const auto total = UtilsOwner::toString2Dec(order.total);
    if (total.empty())
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Invalid total amount"));
        return response;
    }

    try
    {
        // 1. Obtener items
        OrderItemModel orderItemModel;
        auto [items, itemsError] = orderItemModel.getOrderItemsByOrderId(order_id);
        std::cout << "[1] itemsError: " << (int)itemsError << " has_value: " << items.has_value() << std::endl;
        if (itemsError != Errors::NoError || !items.has_value())
        {
            response.set_status_code(web::http::status_codes::NotFound);
            response.set_body(U("Failed to get order items"));
            return response;
        }
        std::cout << "[1] items count: " << items.value().size() << std::endl;

        // 2. Hash
        std::string current_cart_hash = UtilsOwner::hashCart(order.id, order.total, items.value());
        std::cout << "[2] cart_hash: " << current_cart_hash << std::endl;

        // 3. Attempts previos (PENDING o CANCELLED) de esta orden
        PaymentAttempModel paymentAttemptModel;
        InventoryModel inventoryModel;

        auto [attemptsOpt, attemptsError] = paymentAttemptModel.getPendingAttemptsByOrderId(order_id);
        std::cout << "[3] attemptsError: " << (int)attemptsError << " has_value: " << attemptsOpt.has_value() << std::endl;

        std::string idempotencyKey = UtilsOwner::generateUuid();
        bool shouldCreateNewAttempt = true;
        bool shouldReactivateAttempt = false;
        bool shouldReserveStock = true;
        std::string reactivateOldPaypalId = ""; // para el WHERE del UPDATE de reactivación

        if (attemptsError == Errors::NoError && attemptsOpt.has_value())
        {
            std::cout << "[3] attempts count: " << attemptsOpt.value().size() << std::endl;
            for (auto &prevAttempt : attemptsOpt.value())
            {
                if (prevAttempt.cart_hash == current_cart_hash)
                {
                    idempotencyKey = prevAttempt.idempotency_key;
                    shouldCreateNewAttempt = false;

                    if (prevAttempt.status == "PENDING")
                    {
                        std::cout << "[3] mismo carrito, attempt PENDING, reusando sin re-reservar" << std::endl;
                        shouldReserveStock = false;
                    }
                    else // CANCELLED
                    {
                        std::cout << "[3] mismo carrito, attempt CANCELLED, se reactivara a PENDING" << std::endl;
                        shouldReactivateAttempt = true;
                        shouldReserveStock = true; // se habia liberado stock al cancelar, hay que re-reservar
                        reactivateOldPaypalId = prevAttempt.paypal_order_id;
                    }
                }
                else
                {
                    std::cout << "[3] carrito cambio, liberando stock del intento anterior" << std::endl;
                    auto previousItems = UtilsOwner::parseReservedItems(prevAttempt.reserved_items_json);
                    if (!previousItems.empty())
                    {
                        auto [released, releaseError] = inventoryModel.releaseStockForItems(previousItems);
                        std::cout << "[3] releaseError: " << (int)releaseError << std::endl;
                    }
                    if (prevAttempt.status != "CANCELLED")
                    {
                        paymentAttemptModel.updatePaymentAttemptStatus(
                            prevAttempt.paypal_order_id, order_id, user_id, "CANCELLED");
                    }
                }
            }
        }

        // 4. Reservar stock (si hace falta)
        std::cout << "[4] shouldReserveStock: " << shouldReserveStock << std::endl;
        std::string reserved_items_json = "";
        if (shouldReserveStock)
        {
            std::cout << "[4] reservando stock para " << items.value().size() << " items" << std::endl;
            auto [reserved, reserveError] = inventoryModel.reserveStockForItems(items.value());
            std::cout << "[4] reserved: " << reserved << " reserveError: " << (int)reserveError << std::endl;

            if (reserveError == Errors::InsufficientStock)
            {
                response.set_status_code(web::http::status_codes::Conflict);
                response.set_body(U("Stock insuficiente para uno o más productos"));
                return response;
            }
            if (reserveError != Errors::NoError)
            {
                response.set_status_code(web::http::status_codes::InternalError);
                response.set_body(U("Error reserving stock"));
                return response;
            }

            reserved_items_json = UtilsOwner::serializeItemsToJson(items.value());
            std::cout << "[4] reserved_items_json: " << reserved_items_json << std::endl;
        }

        // 5. Llamar a PayPal
        std::cout << "[5] llamando a PayPal..." << std::endl;
        PaypalService paypalService;
        auto paymentResponse = paypalService.createPayment(total, idempotencyKey);
        auto jsonResponse = paymentResponse.extract_json().get();

        std::string newPaypalId = utility::conversions::to_utf8string(
            jsonResponse[U("orderID")].as_string());
        std::string status = jsonResponse[U("status")].as_string();
        std::cout << "[5] PayPal status: " << paymentResponse.status_code() << " newPaypalId: " << newPaypalId << std::endl;

        // 6. Crear, reactivar o dejar el attempt como está
        std::cout << "[6] shouldCreateNewAttempt: " << shouldCreateNewAttempt
                  << " shouldReactivateAttempt: " << shouldReactivateAttempt << std::endl;

        if (shouldCreateNewAttempt)
        {
            auto [attempt, attemptError] = paymentAttemptModel.createPaymentAttempt(
                user_id,
                order_id,
                current_cart_hash,
                order.total,
                idempotencyKey,
                newPaypalId,
                "PENDING",
                reserved_items_json);
            std::cout << "[6] createPaymentAttempt attemptError: " << (int)attemptError << std::endl;
        }
        else if (shouldReactivateAttempt)
        {
            auto [reactivated, reactivateError] = paymentAttemptModel.updatePaymentAttemptStatus(
                reactivateOldPaypalId, order_id, user_id, "PENDING", newPaypalId);
            std::cout << "[6] reactivate attemptError: " << (int)reactivateError << std::endl;
        }
        // si no es ninguno de los dos casos, ya estaba PENDING con el mismo carrito: no hay nada que tocar

        // 7. Actualizar paypal_order_id en la orden si cambió
        std::string storedPaypalId = order.paypal_order_id;
        if (storedPaypalId != newPaypalId)
        {
            orderModel.updateOrderPaypalId(user_id, order_id, newPaypalId);
        }

        web::json::value body;
        body[U("status")] = web::json::value::string(U("success"));
        body[U("paypalResponse")] = jsonResponse;
        response.set_body(body);
        response.set_status_code(paymentResponse.status_code());
    }
    catch (const std::exception &e)
    {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        response.set_status_code(web::http::status_codes::InternalError);
        response.set_body(U("Error processing payment: ") + utility::conversions::to_string_t(e.what()));
    }

    return response;
}
web::http::http_response PaypalController::capturePayment(const web::http::http_request &request, const int user_id)
{

    web::http::http_response response;

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

    auto optOrder = orderModel.getOrderById(order_id, user_id);

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
            auto [orderStatusUpdated, errUpdateOrderStatus] = orderModel.updateOrderStatus(user_id, order_id, "COMPLETED");
            if (errUpdateOrderStatus == Errors::NoError)
            {
                std::cout << "orderStatusUpdated: success" << std::endl;
            }
            else if (errUpdateOrderStatus == Errors::NoRowsAffected)
            {
                std::cout << "orderStatusUpdated: no rows affected" << std::endl;
            }

            auto [paymentAttempStatusUpdated, errUpdatePaymentAttemptStatus] = paymentAttemptModel.updatePaymentAttemptStatus(order_id_paypal, order_id, user_id, "COMPLETED");

            if (errUpdatePaymentAttemptStatus == Errors::NoError)
            {
                std::cout << "paymentAttempStatusUpdated: success" << std::endl;
            }
            else if (errUpdatePaymentAttemptStatus == Errors::NoRowsAffected)
            {
                std::cout << "paymentAttempStatusUpdated: no rows affected" << std::endl;
            }
        }
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

web::http::http_response PaypalController::releasePayment(const web::http::http_request &request, const int user_id)
{
    web::http::http_response response;

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

    try
    {
        PaymentAttempModel paymentAttemptModel;
        InventoryModel inventoryModel;

        // Buscar el attempt PENDING de esta orden
        auto [pendingAttemptsOpt, pendingError] = paymentAttemptModel.getPendingAttemptsByOrderId(order_id);
        if (pendingError != Errors::NoError || !pendingAttemptsOpt.has_value())
        {
            response.set_status_code(web::http::status_codes::NotFound);
            response.set_body(U("No pending attempt found"));
            return response;
        }

        for (auto &attempt : pendingAttemptsOpt.value())
        {
            // Liberar stock usando el snapshot guardado en el attempt
            auto reservedItems = UtilsOwner::parseReservedItems(attempt.reserved_items_json);
            if (!reservedItems.empty())
            {
                auto [released, releaseError] = inventoryModel.releaseStockForItems(reservedItems);
                if (releaseError != Errors::NoError)
                {
                    std::cerr << "Failed to release stock for attempt: " << attempt.id << std::endl;
                }
            }

            // Marcar el attempt como CANCELLED
            paymentAttemptModel.updatePaymentAttemptStatus(
                attempt.paypal_order_id, order_id, user_id, "CANCELLED");
        }

        response.set_status_code(web::http::status_codes::OK);
        web::json::value body;
        body[U("status")] = web::json::value::string(U("success"));
        response.set_body(body);
    }
    catch (const std::exception &e)
    {
        response.set_status_code(web::http::status_codes::InternalError);
        response.set_body(U("Error releasing payment: ") + utility::conversions::to_string_t(e.what()));
    }

    return response;
}