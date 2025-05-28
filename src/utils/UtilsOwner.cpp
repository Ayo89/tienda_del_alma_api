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
static std::string serializeCart(int order_id, double total, const std::vector<OrderItem> &items)
{
    // Copiamos y ordenamos para que el hash no dependa del orden original
    std::vector<OrderItem> sorted = items;
    std::sort(sorted.begin(), sorted.end(),
              [](const OrderItem &a, const OrderItem &b)
              {
                  return a.product_id < b.product_id;
              });

    std::ostringstream oss;
    // Serializar order_id y total (2 decimales)
    oss << order_id << ':'
        << std::fixed << std::setprecision(2) << total
        << ':';

    // Serializar cada ítem en formato "product_id:quantity:price;"
    for (const auto &it : sorted)
    {
        oss << it.product_id << ':'
            << it.quantity << ':'
            << std::fixed << std::setprecision(2) << it.price
            << ';';
    }

    return oss.str();
}

// Función que calcula SHA256 y devuelve hex string
static std::string sha256(const std::string &data)
{
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;

    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (context == nullptr) {
        throw std::runtime_error("Failed to create EVP_MD_CTX");
    }

    if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to initialize digest");
    }

    if (EVP_DigestUpdate(context, data.data(), data.size()) != 1) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to update digest");
    }

    if (EVP_DigestFinal_ex(context, hash, &lengthOfHash) != 1) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to finalize digest");
    }

    EVP_MD_CTX_free(context);

    std::ostringstream oss;
    for (unsigned int i = 0; i < lengthOfHash; ++i)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return oss.str();
}

// Función pública que obtiene el hash del carrito
std::string UtilsOwner::hashCart(int order_id, double total, const std::vector<OrderItem> &items)
{
    std::string serialized = serializeCart(order_id, total, items);
    return sha256(serialized);
}

//sha cart end