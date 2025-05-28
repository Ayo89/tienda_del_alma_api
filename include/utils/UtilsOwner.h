#ifndef UTILSSOWNER_H
#define UTILSOWNER_H
#include <iostream>
#include <string>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <uuid/uuid.h>
#include <openssl/sha.h>
#include <vector>
#include "entities/OrderItem.h"
#include <algorithm>  
#include <openssl/evp.h>



class UtilsOwner
{
public:
    static auto base64_encode(const std::string &input) -> std::string;
    static auto roundToTwoDecimal(double value) -> double;
    static auto toString2Dec(double value) -> std::string;
    static auto generateUuid() -> std::string;
    static std::string hashCart(int order_id, double total, const std::vector<OrderItem> &items);
};
#endif // UTILSSOWNER_H