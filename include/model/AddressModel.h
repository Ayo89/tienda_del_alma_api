#ifndef ADDRESSMODEL_H
#define ADDRESSMODEL_H

#include <string>

class AddressModel
{
public:
    int id;
    std::string street;
    std::string city;
    std::string type; // shipping o billing

    AddressModel(int addressId, std::string streetAddress, std::string cityAddress, std::string addressType)
        : id(addressId), street(streetAddress), city(cityAddress), type(addressType) {}
};

#endif
