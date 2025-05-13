#include "utils/UtilsOwner.h"

auto UtilsOwner::base64_encode(const std::string &input) -> std::string
{
    static const char encode_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encoded;
    int val = 0, valb = -6;
    for (unsigned char c : input)
    {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0)
        {
            encoded.push_back(encode_table[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        encoded.push_back(encode_table[((val << 8) >> (valb + 8)) & 0x3F]);
    while (encoded.size() % 4)
        encoded.push_back('=');
    return encoded;
}

auto UtilsOwner::roundToTwoDecimal(double value) -> double
{
    return std::round(value * 100.0) / 100.0;
}

auto UtilsOwner::toString2Dec(double value) -> std::string
{
    std::ostringstream oss;
    oss << std::fixed
        << std::setprecision(2)
        << value;
    return oss.str();
}

auto UtilsOwner::generateUuid() -> std::string
{
    uuid_t uuid;
    uuid_generate_random(uuid);
    char uuid_str[37];
    uuid_unparse_lower(uuid, uuid_str);
    return std::string(uuid_str);
}


// sha cart start
static std::string serializeCart(const std::vector<OrderItem> &items)
{
    std::ostringstream oss;
    for (const auto &it : items)
    {
        // Formato: "id:qty:price;"
        oss << it.product_id << ':' << it.quantity << ':' << std::fixed << std::setprecision(2) << it.price << ';';
    }
    return oss.str();
}

// Función que calcula SHA256 y devuelve hex string
static std::string sha256(const std::string &data)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.data(), data.size());
    SHA256_Final(hash, &sha256);

    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return oss.str();
}

// Función pública que obtiene el hash del carrito
std::string UtilsOwner::hashCart(const std::vector<OrderItem> &items)
{
    std::string serialized = serializeCart(items);
    return sha256(serialized);
}

//sha cart end