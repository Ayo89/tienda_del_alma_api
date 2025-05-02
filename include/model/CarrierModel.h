#ifndef CARRIERMODEL_H
#define CARRIERMODEL_H
#include "db/DatabaseConnection.h"
#include "entities/Carrier.h"
#include <optional>
#include <vector>
#include <cstring>
#include "utils/Errors.h"
#include <memory>
#include <string>

class CarrierModel
{
private:
    /* data */
public:
    CarrierModel();
    bool insertSampleCarriers();
    std::pair<std::optional<std::vector<Carrier>>, Errors> getAllCarriers();
    std::pair<std::optional<Carrier>, Errors> getCarrierById(int &id);
};

#endif