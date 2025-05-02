#include "controllers/OrderController.h"
#include "model/OrderModel.h" // Make sure to include the model

OrderController::OrderController() {}

web::http::http_response OrderController::createOrder(const web::http::http_request &request)
{
    web::http::http_response response;

    // 1. Obtain user_id from JWT token
    auto optUserId = AuthUtils::getUserIdFromRequest(request);

    // If user_id is not available in the JWT token, return Unauthorized
    if (!optUserId)
    {
        response.set_status_code(web::http::status_codes::Unauthorized);
        response.set_body(U("Unauthorized"));
        return response;
    }
    int user_id;
    try
    {
        user_id = std::stoi(optUserId.value()); // Convert user_id to integer
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
        body = request.extract_json().get(); // Extract and parse JSON body from the request
    }
    catch (const std::exception &)
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Invalid JSON body"));
        return response;
    }
    std::cout << body.has_field(U("products")) << std::endl; // Debugging line to print the JSON body
    // 3. Validate required fields in the body
    if (!body.has_field(U("shipping_address_id")) ||
        !body.has_field(U("billing_address_id")) ||
        !body.has_field(U("products")))
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Missing required fields: shipping_address_id, billing_address_id, products"));
        return response;
    }

    int shipping_address_id = body[U("shipping_address_id")].as_integer();
    int billing_address_id = body[U("billing_address_id")].as_integer();

    // 4. Parse products (items) from the JSON array
    std::vector<OrderItem> products;

    try
    {
        auto arr = body[U("products")].as_array();
        for (const auto &item : arr)
        {
            OrderItem prod;
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

    // 5. Optional fields (e.g., shipment date, delivery date, payment method, etc.)
    auto getOpt = [&](const utility::string_t &key)
    {
        return body.has_field(key) ? body[key].as_string() : U(""); // Return empty string if the key is not found
    };
    std::string shipment_date = getOpt(U("shipment_date"));
    std::string delivery_date = getOpt(U("delivery_date"));
    int carrier_id = body.has_field(U("carrier_id")) ? std::stoi(body[U("carrier_id")].as_string()) : 1;
    std::string tracking_url = getOpt(U("tracking_url"));
    std::string tracking_number = getOpt(U("tracking_number"));
    std::string payment_method = getOpt(U("payment_method"));
    std::string payment_status = getOpt(U("payment_status"));

    // 6. Call the model to create or update the order
    OrderModel orderModel;
    OrderItemModel orderItemModel;
    std::optional<Order> optOrder = model.getPendingOrderByUserId(user_id); // Check if there is an existing pending order for the user

    if (optOrder.has_value()) // If the order exists, update it
    {
        Order &existingOrder = optOrder.value();

        auto [updatedOrder, error] = orderModel.updateOrder(
            user_id,
            existingOrder.id,
            shipping_address_id,
            billing_address_id,
            products,
            shipment_date,
            delivery_date,
            carrier_id,
            tracking_url,
            tracking_number,
            payment_method,
            payment_status);

        std::optional<int> optOrderItemId = orderItemModel.syncOrderItems(products, existingOrder.id); // Sync products with the order

        web::json::value respBody;
        if (error == Errors::NoError || error == Errors::NoRowsAffected)
        {
            response.set_status_code(web::http::status_codes::OK);
            respBody[U("order_id")] = web::json::value::number(updatedOrder.value().id);

            if (error == Errors::NoRowsAffected)
            {
                respBody[U("message")] = web::json::value::string(U("No changes made to the order"));
            }
            else
            {
                respBody[U("message")] = web::json::value::string(U("Order updated successfully"));
            }

            response.headers().add(U("Content-Type"), U("application/json"));
            response.set_body(respBody);
            return response;
        }
        else
        {

            response.set_status_code(web::http::status_codes::InternalError);
            response.set_body(U("Error updating order"));
            return response;
        }
    }

    else
    {
        std::cout << "carrier_id" << carrier_id << std::endl;
        // Order does not exist, create a new order
        std::optional<int> optOrderId = model.createOrder(
            user_id,
            shipping_address_id,
            billing_address_id,
            products,
            shipment_date,
            delivery_date,
            carrier_id,
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

    // 1. Obtain user_id from JWT token
    std::optional<std::string> optUserId = AuthUtils::getUserIdFromRequest(request);
    if (!optUserId.has_value()) // If token is not provided or is invalid
    {
        response.set_status_code(web::http::status_codes::Unauthorized);
        response.set_body(U("Token not provided or invalid"));
        return response;
    }
    int user_id = std::stoi(optUserId.value());
    OrderModel model;
    auto optOrders = model.getOrdersByUserId(user_id); // Get orders for the specified user
    if (!optOrders.has_value())                        // If no orders are found
    {
        response.set_status_code(web::http::status_codes::NotFound);
        response.set_body(U("No orders found for the user"));
        return response;
    }

    response.set_status_code(web::http::status_codes::OK);

    if (optOrders.has_value() && !optOrders->empty()) // If orders are found
    {
        web::json::value json_response = web::json::value::array();
        for (size_t i = 0; i < optOrders->size(); ++i) // Loop through each order
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
            json_orders[U("carrier_id")] = web::json::value::number(order.carrier_id);
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
        // If no orders are found, return a message
        web::json::value empty_msg = web::json::value::object();
        empty_msg[U("message")] = web::json::value::string(U("No orders found for this user"));
        response.set_body(empty_msg);
    }
    return response;
}

web::http::http_response OrderController::getOrderById(const web::http::http_request &request)
{
    web::http::http_response response;

    // 1. Obtain user_id from JWT token
    std::optional<std::string> optUserId = AuthUtils::getUserIdFromRequest(request);
    if (!optUserId.has_value()) // If token is not provided or is invalid
    {
        response.set_status_code(web::http::status_codes::Unauthorized);
        response.set_body(U("Token not provided or invalid"));
        return response;
    }
    int user_id = std::stoi(optUserId.value());

    // 2. Get order_id from the request URI
    auto uri = request.request_uri();
    auto path = uri.path();
    if (path.empty() || path.find_last_of('/') == std::string::npos)
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Invalid request URI"));
        return response;
    }
    // Extract the order_id from the URI
    // Assuming the URI is like /orders/{order_id}
    auto address_id_str = path.substr(path.find_last_of('/') + 1);
    int order_id = std::stoi(address_id_str);

    // 3. Call the model to get the order by ID
    OrderModel model;
    auto optOrder = model.getOrderById(user_id, order_id); // Get the order for the specified user and order ID
    if (!optOrder.has_value())                             // If no order is found
    {
        response.set_status_code(web::http::status_codes::NotFound);
        response.set_body(U("Order not found"));
        return response;
    }

    // 4. Prepare the JSON response
    const auto &order = optOrder.value();
    web::json::value json_order;
    json_order[U("order_id")] = web::json::value::number(order.id);
    json_order[U("shipping_address_id")] = web::json::value::number(order.shipping_address_id);
    json_order[U("billing_address_id")] = web::json::value::number(order.billing_address_id);
    json_order[U("status")] = web::json::value::string(order.status);
    json_order[U("total")] = web::json::value::number(order.total);
    json_order[U("shipment_date")] = web::json::value::string(order.shipment_date);
    json_order[U("delivery_date")] = web::json::value::string(order.delivery_date);
    json_order[U("carrier_id")] = web::json::value::number(order.carrier_id);
    json_order[U("tracking_url")] = web::json::value::string(order.tracking_url);
    json_order[U("tracking_number")] = web::json::value::string(order.tracking_number);
    json_order[U("payment_method")] = web::json::value::string(order.payment_method);
    json_order[U("payment_status")] = web::json::value::string(order.payment_status);
    response.set_body(json_order);
    return response;
}

web::http::http_response OrderController::updateTotalbyOrderId(const web::http::http_request &request)
{
    web::http::http_response response;

    // 1. Obtain user_id from JWT token
    std::optional<std::string> optUserId = AuthUtils::getUserIdFromRequest(request);
    if (!optUserId.has_value()) // If token is not provided or is invalid
    {
        response.set_status_code(web::http::status_codes::Unauthorized);
        response.set_body(U("Token not provided or invalid"));
        return response;
    }
    int user_id = std::stoi(optUserId.value());

    // 2. Get carrier_id from the request URI
    auto uri = request.request_uri(); // Extract the URI from the request
    auto path_segments = web::uri::split_path(uri.path());
    if (path_segments.size() != 4 || path_segments[0] != U("order") || path_segments[2] != U("carrier"))
    {
        response.set_status_code(web::http::status_codes::BadRequest);
        response.set_body(U("Invalid URI format. Expected: /order/{id}/carrier/{id}"));
        return response;
    }

    int order_id = std::stoi(path_segments[1]);
    int carrier_id = std::stoi(path_segments[3]);
    std::cout << "order_id: " << order_id << std::endl;
    std::cout << "carrier_id: " << carrier_id << std::endl;
    // 3. Call de model to get Carrier by ID
    CarrierModel carrierModel;

    auto [optCarrier, errorsGetCarrier] = carrierModel.getCarrierById(carrier_id);
    std::cout << "Carrier price: " << optCarrier->price << std::endl;
    if (!optCarrier.has_value())
    {
        response.set_status_code(web::http::status_codes::NotFound);
        response.set_body(U("No carriers found"));
        return response;
    }

    // 4 Call the model to get the order by ID
    OrderModel orderModel;
    auto optOrder = orderModel.getOrderById(user_id, order_id); // Get the order for the specified user and order ID
    if (!optOrder.has_value())                                  // If no order is found
    {
        response.set_status_code(web::http::status_codes::NotFound);
        response.set_body(U("Order not found"));
        return response;
    }

    auto [updCarrierId, errors] = orderModel.updateCarrierId(order_id, carrier_id);
    if (!updCarrierId)
    {
        response.set_status_code(web::http::status_codes::InternalError);
        response.set_body(U("Failed to update carrier id"));
        return response;
    }

    double newTotal = optOrder->total + optCarrier->price;

    // 5 call updateOrderTotal
    auto [result, errorsUpdateOrderTotal] = orderModel.updateOrderTotal(order_id, newTotal);
    if (!result)
    {
        response.set_status_code(web::http::status_codes::InternalError);
        response.set_body(U("Failed to update order total"));
        return response;
    }
    response.set_status_code(web::http::status_codes::OK);
    // 6. Prepare the JSON response
    web::json::value json_order;
    json_order[U("order_id")] = web::json::value::number(order_id);
    json_order[U("total")] = web::json::value::number(newTotal);
    json_order[U("update: success")] = web::json::value::boolean(true);
    response.set_body(json_order);
    return response;
}