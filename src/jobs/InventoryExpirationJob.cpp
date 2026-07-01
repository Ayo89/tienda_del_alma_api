#include "jobs/InventoryExpirationJob.h"
#include "model/PaymentAttemptModel.h"
#include "model/InventoryModel.h"
#include "utils/UtilsOwner.h"
#include <thread>
#include <chrono>
#include <iostream>

void runExpirationCheck()
{
    PaymentAttempModel paymentAttemptModel;
    InventoryModel inventoryModel;

    auto [expiredAttempts, error] = paymentAttemptModel.getExpiredPendingAttempts(5); // 5 minutos

    if (error != Errors::NoError)
    {
        std::cerr << "[ExpirationJob] Error obteniendo intentos expirados" << std::endl;
        return;
    }

    if (!expiredAttempts.has_value())
    {
        return;
    }

    for (auto &attempt : expiredAttempts.value())
    {
        // Usamos el snapshot guardado en el attempt, no los items actuales de la orden,
        // porque el carrito puede haber cambiado desde que se reservó este stock.
        auto reservedItems = UtilsOwner::parseReservedItems(attempt.reserved_items_json);

        if (reservedItems.empty())
        {
            std::cerr << "[ExpirationJob] reserved_items vacío para attempt id: "
                       << attempt.id << ", order_id: " << attempt.order_id << std::endl;
            continue;
        }

        auto [released, releaseError] = inventoryModel.releaseStockForItems(reservedItems);
        if (released && releaseError == Errors::NoError)
        {
            std::string paypalOrderId = attempt.paypal_order_id; // updatePaymentAttemptStatus toma referencia no-const
            auto [updated, updateError] = paymentAttemptModel.updatePaymentAttemptStatus(
                paypalOrderId, attempt.order_id, attempt.user_id, "EXPIRED");

            if (updateError == Errors::NoError)
            {
                std::cout << "[ExpirationJob] Stock liberado y attempt EXPIRED para order_id: "
                           << attempt.order_id << std::endl;
            }
            else
            {
                std::cerr << "[ExpirationJob] Stock liberado pero fallo al actualizar status para order_id: "
                           << attempt.order_id << " (error: " << (int)updateError << ")" << std::endl;
            }
        }
        else
        {
            std::cerr << "[ExpirationJob] Falló liberar stock para order_id: "
                       << attempt.order_id << " (releaseError: " << (int)releaseError << ")" << std::endl;
        }
    }
}

void startInventoryExpirationJob()
{
    std::thread([]() {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::minutes(1));
            try
            {
                runExpirationCheck();
            }
            catch (const std::exception &e)
            {
                std::cerr << "[ExpirationJob] Excepción: " << e.what() << std::endl;
            }
        }
    }).detach();
}