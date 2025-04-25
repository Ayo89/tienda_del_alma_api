#include "controllers/OrderController.h"
#include "model/OrderModel.h" // Asegúrate de incluir el modelo

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
        response.set_body(U("Missing required fields: shipping_address_id, billing_address_id, total, products"));
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

    // 6. Call the model to create or update the order
    OrderModel model;
    std::optional<Order> optOrder = model.getPendingOrderByUserId(user_id); 
    std::cout << "optOrder: " << optOrder.has_value()  << std::endl;
    if (optOrder.has_value())
    {
        // Order exists, update it
        Order &existingOrder = optOrder.value(); // Obtener la orden existente
        std::cout << existingOrder.id << std::endl;
        std::optional<Order> updatedOrder = model.updateOrder( // Cambiado a Order
            user_id,
            existingOrder.id,
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

        if (!updatedOrder)
        {
            response.set_status_code(web::http::status_codes::InternalError);
            response.set_body(U("Unable to update existing pending order"));
            return response;
        }

        response.set_status_code(web::http::status_codes::OK);
        web::json::value respBody;
        respBody[U("order_id")] = web::json::value::number(updatedOrder.value().id); // Usar el ID de la orden actualizada
        respBody[U("message")] = web::json::value::string(U("Order updated successfully"));
        response.headers().add(U("Content-Type"), U("application/json"));
        response.set_body(respBody);
        return response;
    }
    else
    {
        // Order does not exist, create it
        std::optional<int> optOrderId = model.createOrder(
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

        web::json::value respBody;
        respBody[U("order_id")] = web::json::value::number(optOrderId.value());
        respBody[U("message")] = web::json::value::string(U("Order created successfully"));

        response.set_status_code(web::http::status_codes::Created);
        response.headers().add(U("Content-Type"), U("application/json"));
        response.set_body(respBody);
        return response;
    }
}

web::http::http_response OrderController::getOrdersByUserId(const web::http::http_request &request)
{
    web::http::http_response response;
    std::optional<std::string> optUserId = AuthUtils::getUserIdFromRequest(request);
    if (!optUserId.has_value())
    {
        response.set_status_code(web::http::status_codes::Unauthorized);
        response.set_body(U("Token no proporcionado o inválido"));
        return response;
    }
    int user_id = std::stoi(optUserId.value());
    OrderModel model;
    auto optOrders = model.getOrdersByUserId(user_id);
    if (!optOrders.has_value())
    {
        response.set_status_code(web::http::status_codes::NotFound);
        response.set_body(U("No orders found for the user"));
        return response;
    }
    response.set_status_code(web::http::status_codes::OK);

    if (optOrders.has_value() && !optOrders->empty())
    {
        web::json::value json_response = web::json::value::array();
        for (size_t i = 0; i < optOrders->size(); ++i)
        {
            const auto &order = optOrders->at(i);
            web::json::value json_orders;
            json_orders[U("order_id")] = web::json::value::number(order.id);
            json_orders[U("shipping_address_id")] = web::json::value::number(order.shipping_address_id);
            json_orders[U("billing_address_id")] = web::json::value::number(order.billing_address_id);
            json_orders[U("status")] = web::json::value::string(order.status);
            json_orders[U("total")] = web::json::value::number(order.total);
            json_orders[U("shipment_date")] = web::json::value::string(order.shipment_date);
            json_orders[U("delivery_date")] = web::json::value::string(order.delivery_date);
            json_orders[U("carrier")] = web::json::value::string(order.carrier);
            json_orders[U("tracking_url")] = web::json::value::string(order.tracking_url);
            json_orders[U("tracking_number")] = web::json::value::string(order.tracking_number);
            json_orders[U("payment_method")] = web::json::value::string(order.payment_method);
            json_orders[U("payment_status")] = web::json::value::string(order.payment_status);
            json_response[i] = json_orders;
        }
        response.set_body(json_response);
    }
    else
    {
        web::json::value empty_msg = web::json::value::object();
        empty_msg[U("message")] = web::json::value::string(U("Not found orders for this user"));
        response.set_body(empty_msg);
    }
    return response;
}
