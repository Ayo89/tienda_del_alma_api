#ifndef ADDRESSMODEL_H
#define ADDRESSMODEL_H

#include <string>
#include <vector>
#include <optional>
#include "entities/Address.h"
#include "db/DatabaseConnection.h"
#include "db/DatabaseInitializer.h"

class AddressModel
{
private:
    DatabaseConnection &db;

public:
    AddressModel(DatabaseConnection &database) : db(database) {}
    std::optional<int> createAddress(
        const int &user_id,
        const std::string &first_name,
        const std::string &last_name,
        const std::string &phone,
        const std::string &street,
        const std::string &city,
        const std::string &province,
        const std::string &postal_code,
        const std::string &country,
        const std::string &type = "",
        const bool &is_default = false,
        const std::string &additional_info = "");

    std::optional<std::vector<Address>> getAllAddressByUserId(const int &user_id);

    std::optional<Address> getAddressById(const int &address_id, const int &user_id, const std::string &type = "");

    std::optional<int> updateAddress(
        const int &user_id,
        const int &address_id,
        const std::string &first_name,
        const std::string &last_name,
        const std::string &phone,
        const std::string &street,
        const std::string &city,
        const std::string &province,
        const std::string &postal_code,
        const std::string &country,
        const bool &is_default = false,
        const std::string &type = "",
        const std::string &additional_info = "");
};

#endif
