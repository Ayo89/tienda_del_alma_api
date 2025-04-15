#include "model/AddressModel.h"
#include <mysql/mysql.h>
#include <cstring>
#include <iostream>
#include <memory>

//---------------->>CREATE ADDRESS<<------------------//
std::optional<int> AddressModel::createAddress(
    const int &user_id,
    const std::string &first_name,
    const std::string &last_name,
    const std::string &phone,
    const std::string &street,
    const std::string &city,
    const std::string &province,
    const std::string &postal_code,
    const std::string &country,
    const std::string &type,
    const bool &is_default,
    const std::string &additional_info)
{
    // Validaciones básicas (additional_info es opcional)
    if (user_id == 0 || first_name.empty() || last_name.empty() || phone.empty() ||
        street.empty() || city.empty() || province.empty() || postal_code.empty() || country.empty())
    {
        std::cerr << "Error: Required fields cannot be empty" << std::endl;
        return std::nullopt;
    }

    // Obtener el puntero a la conexión MySQL
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Error: No active database connection" << std::endl;
        return std::nullopt;
    }
    const char *query = nullptr;
    // Preparar la consulta
    if (type == "billing")
    {
        query = "INSERT INTO billing_addresses (user_id, first_name, last_name, phone, street, city, province, postal_code, country, is_default, additional_info) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    }
    else
    {
        query = "INSERT INTO shipping_addresses (user_id, first_name, last_name, phone, street, city, province, postal_code, country, is_default, additional_info) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    }

    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Statement initialization failed: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cerr << "Statement preparation failed: " << mysql_stmt_error(stmt)
                  << " (MySQL error: " << mysql_error(conn) << ")" << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    // Variable local para manejar el valor NULL (si es necesario)
    bool is_null_additional = additional_info.empty();

    // Vincular parámetros
    MYSQL_BIND bind[11];
    memset(bind, 0, sizeof(bind));

    char first_name_buf[256];
    strncpy(first_name_buf, first_name.c_str(), sizeof(first_name_buf) - 1);
    first_name_buf[sizeof(first_name_buf) - 1] = '\0';
    unsigned long first_name_length = strlen(first_name_buf);

    char last_name_buf[256];
    strncpy(last_name_buf, last_name.c_str(), sizeof(last_name_buf) - 1);
    last_name_buf[sizeof(last_name_buf) - 1] = '\0';
    unsigned long last_name_length = strlen(last_name_buf);

    char phone_buf[256];
    strncpy(phone_buf, phone.c_str(), sizeof(phone_buf) - 1);
    phone_buf[sizeof(phone_buf) - 1] = '\0';
    unsigned long phone_length = strlen(phone_buf);

    char street_buf[256];
    strncpy(street_buf, street.c_str(), sizeof(street_buf) - 1);
    street_buf[sizeof(street_buf) - 1] = '\0';
    unsigned long street_length = strlen(street_buf);

    char city_buf[256];
    strncpy(city_buf, city.c_str(), sizeof(city_buf) - 1);
    city_buf[sizeof(city_buf) - 1] = '\0';
    unsigned long city_length = strlen(city_buf);

    char province_buf[256];
    strncpy(province_buf, province.c_str(), sizeof(province_buf) - 1);
    province_buf[sizeof(province_buf) - 1] = '\0';
    unsigned long province_length = strlen(province_buf);

    char postal_code_buf[256];
    strncpy(postal_code_buf, postal_code.c_str(), sizeof(postal_code_buf) - 1);
    postal_code_buf[sizeof(postal_code_buf) - 1] = '\0';
    unsigned long postal_code_length = strlen(postal_code_buf);

    char country_buf[256];
    strncpy(country_buf, country.c_str(), sizeof(country_buf) - 1);
    country_buf[sizeof(country_buf) - 1] = '\0';
    unsigned long country_length = strlen(country_buf);

    char additional_info_buf[1000];
    strncpy(additional_info_buf, additional_info.c_str(), sizeof(additional_info_buf) - 1);
    additional_info_buf[sizeof(additional_info_buf) - 1] = '\0';
    unsigned long additional_info_length = strlen(additional_info_buf);

    // user_id
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (void *)&user_id;
    bind[0].is_unsigned = false;

    // first_name
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void *)first_name_buf;
    bind[1].buffer_length = sizeof(first_name_buf);
    bind[1].length = &first_name_length;
    bind[1].is_null = nullptr;

    // last_name
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (void *)last_name_buf;
    bind[2].buffer_length = sizeof(last_name_buf);
    bind[2].length = &last_name_length;
    bind[2].is_null = nullptr;

    // phone
    bind[3].buffer_type = MYSQL_TYPE_STRING;
    bind[3].buffer = (void *)phone_buf;
    bind[3].buffer_length = sizeof(phone_buf);
    bind[3].length = &phone_length;
    bind[3].is_null = nullptr;

    // street
    bind[4].buffer_type = MYSQL_TYPE_STRING;
    bind[4].buffer = (void *)street_buf;
    bind[4].buffer_length = sizeof(street_buf);
    bind[4].length = &street_length;
    bind[4].is_null = nullptr;

    // city
    bind[5].buffer_type = MYSQL_TYPE_STRING;
    bind[5].buffer = (void *)city_buf;
    bind[5].buffer_length = sizeof(city_buf);
    bind[5].length = &city_length;
    bind[5].is_null = nullptr;

    // province
    bind[6].buffer_type = MYSQL_TYPE_STRING;
    bind[6].buffer = (void *)province_buf;
    bind[6].buffer_length = sizeof(province_buf);
    bind[6].length = &province_length;
    bind[6].is_null = nullptr;

    // postal_code
    bind[7].buffer_type = MYSQL_TYPE_STRING;
    bind[7].buffer = (void *)postal_code_buf;
    bind[7].buffer_length = sizeof(postal_code_buf);
    bind[7].length = &postal_code_length;
    bind[7].is_null = nullptr;

    // country
    bind[8].buffer_type = MYSQL_TYPE_STRING;
    bind[8].buffer = (void *)country_buf;
    bind[8].buffer_length = sizeof(country_buf);
    bind[8].length = &country_length;
    bind[8].is_null = nullptr;

    // is_default
    unsigned char is_default_char = is_default ? 1 : 0;
    bind[9].buffer_type = MYSQL_TYPE_TINY;
    bind[9].buffer = (void *)&is_default_char;
    bind[9].buffer_length = sizeof(is_default_char);
    bind[9].is_null = nullptr;

    // additional_info
    bind[10].buffer_type = MYSQL_TYPE_STRING;
    bind[10].buffer = (void *)additional_info_buf;
    bind[10].buffer_length = sizeof(additional_info_buf);
    bind[10].length = &additional_info_length;
    bind[10].is_null = &is_null_additional;

    // Vincular los parámetros
    if (mysql_stmt_bind_param(stmt, bind) != 0)
    {
        std::cerr << "Parameter binding failed: " << mysql_stmt_error(stmt)
                  << " (MySQL error: " << mysql_error(conn) << ")" << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    // Ejecutar la consulta preparada
    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Execution failed: " << mysql_stmt_error(stmt)
                  << " (MySQL error: " << mysql_error(conn) << ")" << std::endl;
        mysql_stmt_close(stmt);
        return std::nullopt;
    }

    // Obtener el ID de la dirección creada
    int address_id = mysql_insert_id(conn);

    std::cout << "Address created successfully for user_id: " << user_id << std::endl;
    mysql_stmt_close(stmt);
    return address_id;
}

//---------------->>GET ALL ADDRESSES BY USER ID<<------------------//
std::optional<std::vector<Address>> AddressModel::getAllAddressByUserId(const int &user_id)
{
    MYSQL *conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0)
    {
        std::cerr << "Error: No active database connection: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    std::vector<Address> addresses;
    const char *query =
        "SELECT id, first_name, last_name, phone, street, city, province, postal_code, country, is_default, additional_info, 'billing' AS type "
        "FROM billing_addresses WHERE user_id = ? "
        "UNION "
        "SELECT id, first_name, last_name, phone, street, city, province, postal_code, country, is_default, additional_info, 'shipping' AS type "
        "FROM shipping_addresses WHERE user_id = ?";

    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Statement initialization failed: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }
    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);

    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cerr << "Statement preparation failed: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }
    std::cout << "Query prepared successfully" << std::endl;

    MYSQL_BIND param[2];
    memset(param, 0, sizeof(param));
    param[0].buffer_type = MYSQL_TYPE_LONG;
    param[0].buffer = (void *)&user_id;
    param[1].buffer_type = MYSQL_TYPE_LONG;
    param[1].buffer = (void *)&user_id;

    if (mysql_stmt_bind_param(stmt, param) != 0 || mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Execution failed: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }

    MYSQL_BIND result[12];
    memset(result, 0, sizeof(result));

    // Define local variables to hold result data
    int id;
    char first_name[255], last_name[255], phone[50], street[255], city[100],
        province[100], postal_code[20], country[100], additional_info[1000], type[20];
    char is_default;
    unsigned long len[12];
    bool is_null[12];

    // Bind each column from the query result

    // id
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &id;
    result[0].is_null = &is_null[0];

    // first_name
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = first_name;
    result[1].buffer_length = sizeof(first_name);
    result[1].length = &len[1];
    result[1].is_null = &is_null[1];

    // last_name
    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = last_name;
    result[2].buffer_length = sizeof(last_name);
    result[2].length = &len[2];
    result[2].is_null = &is_null[2];

    // phone
    result[3].buffer_type = MYSQL_TYPE_STRING;
    result[3].buffer = phone;
    result[3].buffer_length = sizeof(phone);
    result[3].length = &len[3];
    result[3].is_null = &is_null[3];

    // street
    result[4].buffer_type = MYSQL_TYPE_STRING;
    result[4].buffer = street;
    result[4].buffer_length = sizeof(street);
    result[4].length = &len[4];
    result[4].is_null = &is_null[4];

    // city
    result[5].buffer_type = MYSQL_TYPE_STRING;
    result[5].buffer = city;
    result[5].buffer_length = sizeof(city);
    result[5].length = &len[5];
    result[5].is_null = &is_null[5];

    // province
    result[6].buffer_type = MYSQL_TYPE_STRING;
    result[6].buffer = province;
    result[6].buffer_length = sizeof(province);
    result[6].length = &len[6];
    result[6].is_null = &is_null[6];

    // postal_code
    result[7].buffer_type = MYSQL_TYPE_STRING;
    result[7].buffer = postal_code;
    result[7].buffer_length = sizeof(postal_code);
    result[7].length = &len[7];
    result[7].is_null = &is_null[7];

    // country
    result[8].buffer_type = MYSQL_TYPE_STRING;
    result[8].buffer = country;
    result[8].buffer_length = sizeof(country);
    result[8].length = &len[8];
    result[8].is_null = &is_null[8];

    // is_default
    result[9].buffer_type = MYSQL_TYPE_TINY;
    result[9].buffer = &is_default;
    result[9].is_null = &is_null[9];

    // additional_info
    result[10].buffer_type = MYSQL_TYPE_STRING;
    result[10].buffer = additional_info;
    result[10].buffer_length = sizeof(additional_info);
    result[10].length = &len[10];
    result[10].is_null = &is_null[10];

    // type
    result[11].buffer_type = MYSQL_TYPE_STRING;
    result[11].buffer = type;
    result[11].buffer_length = sizeof(type);
    result[11].length = &len[11];
    result[11].is_null = &is_null[11];

    // Bind the result buffers to the statement
    if (mysql_stmt_bind_result(stmt, result) != 0)
    {
        std::cerr << "❌ Result binding failed: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }

    std::cout << "✅ Result binding successful" << std::endl;

    // Fetch the results one by one
    int fetch_result;
    while ((fetch_result = mysql_stmt_fetch(stmt)) == 0)
    {
        Address address;
        address.id = id;
        address.first_name = is_null[1] ? "" : std::string(first_name, len[1]);
        address.last_name = is_null[2] ? "" : std::string(last_name, len[2]);
        address.phone = is_null[3] ? "" : std::string(phone, len[3]);
        address.street = is_null[4] ? "" : std::string(street, len[4]);
        address.city = is_null[5] ? "" : std::string(city, len[5]);
        address.province = is_null[6] ? "" : std::string(province, len[6]);
        address.postal_code = is_null[7] ? "" : std::string(postal_code, len[7]);
        address.country = is_null[8] ? "" : std::string(country, len[8]);
        address.is_default = is_default;
        address.additional_info = is_null[10] ? "" : std::string(additional_info, len[10]);
        address.type = is_null[11] ? "" : std::string(type, len[11]);

        addresses.push_back(address);
    }

    // Check fetch result after the loop
    if (fetch_result == MYSQL_NO_DATA)
    {
        std::cout << "⚠️ No addresses found for user_id: " << user_id << std::endl;
        return addresses; // Return an empty vector
    }
    else if (fetch_result != 0)
    {
        std::cerr << "❌ Error in mysql_stmt_fetch: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }

    std::cout << "✅ Total addresses found: " << addresses.size() << std::endl;
    return addresses;
}
//---------------->>GET ADDRESS BY ID<<------------------//
/* std::optional<int> AddressModel::updateAddress(
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
    const std::string &type = "",
    const bool &is_default = false,
    const std::string &additional_info = "")
{
    // Obtener la conexión a la base de datos
    MYSQL *conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0)
    {
        fprintf(stderr, "Error: No active database connection: %s\n", mysql_error(conn));
        return std::nullopt;
    }

    // Sentencia SQL para actualizar la dirección
    // Se actualizan los campos y se filtra por el id de la dirección y el user_id (por seguridad)
    const char *query =
        "UPDATE addresses "
        "SET first_name = ?, last_name = ?, phone = ?, street = ?, city = ?, province = ?, postal_code = ?, country = ?, type = ?, is_default = ?, additional_info = ? "
        "WHERE id = ? AND user_id = ?";

    // Inicializar la sentencia
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        fprintf(stderr, "Statement initialization failed: %s\n", mysql_error(conn));
        return std::nullopt;
    }
    // Utilizamos RAII para garantizar que se cierre el statement
    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);

    // Preparar la sentencia SQL
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        fprintf(stderr, "Statement preparation failed: %s\n", mysql_stmt_error(stmt));
        return std::nullopt;
    }

    // Declarar el arreglo para los parámetros. La cantidad total de parámetros en la consulta es 13
    constexpr size_t NUM_PARAMS = 13;
    MYSQL_BIND param[NUM_PARAMS];
    memset(param, 0, sizeof(param));

    // Definir los parámetros en el mismo orden en el que aparecen en la consulta

    // 1. first_name
    param[0].buffer_type = MYSQL_TYPE_STRING;
    param[0].buffer = (void *)first_name.c_str();
    param[0].buffer_length = first_name.size();

    // 2. last_name
    param[1].buffer_type = MYSQL_TYPE_STRING;
    param[1].buffer = (void *)last_name.c_str();
    param[1].buffer_length = last_name.size();

    // 3. phone
    param[2].buffer_type = MYSQL_TYPE_STRING;
    param[2].buffer = (void *)phone.c_str();
    param[2].buffer_length = phone.size();

    // 4. street
    param[3].buffer_type = MYSQL_TYPE_STRING;
    param[3].buffer = (void *)street.c_str();
    param[3].buffer_length = street.size();

    // 5. city
    param[4].buffer_type = MYSQL_TYPE_STRING;
    param[4].buffer = (void *)city.c_str();
    param[4].buffer_length = city.size();

    // 6. province
    param[5].buffer_type = MYSQL_TYPE_STRING;
    param[5].buffer = (void *)province.c_str();
    param[5].buffer_length = province.size();

    // 7. postal_code
    param[6].buffer_type = MYSQL_TYPE_STRING;
    param[6].buffer = (void *)postal_code.c_str();
    param[6].buffer_length = postal_code.size();

    // 8. country
    param[7].buffer_type = MYSQL_TYPE_STRING;
    param[7].buffer = (void *)country.c_str();
    param[7].buffer_length = country.size();

    // 9. type
    param[8].buffer_type = MYSQL_TYPE_STRING;
    param[8].buffer = (void *)type.c_str();
    param[8].buffer_length = type.size();

    // 10. is_default
    // Convertir bool a un unsigned char (o TINYINT) ya que MySQL espera un entero en este caso.
    unsigned char is_default_val = is_default ? 1 : 0;
    param[9].buffer_type = MYSQL_TYPE_TINY;
    param[9].buffer = (void *)&is_default_val;
    param[9].buffer_length = sizeof(is_default_val);

    // 11. additional_info
    param[10].buffer_type = MYSQL_TYPE_STRING;
    param[10].buffer = (void *)additional_info.c_str();
    param[10].buffer_length = additional_info.size();

    // 12. address_id (ID de la dirección a actualizar)
    param[11].buffer_type = MYSQL_TYPE_LONG;
    param[11].buffer = (void *)&address_id;
    param[11].buffer_length = sizeof(address_id);

    // 13. user_id (para asegurarse de que la dirección pertenece al usuario)
    param[12].buffer_type = MYSQL_TYPE_LONG;
    param[12].buffer = (void *)&user_id;
    param[12].buffer_length = sizeof(user_id);

    // Enlazar los parámetros a la sentencia
    if (mysql_stmt_bind_param(stmt, param) != 0)
    {
        fprintf(stderr, "Parameter binding failed: %s\n", mysql_stmt_error(stmt));
        return std::nullopt;
    }

    // Ejecutar la sentencia
    if (mysql_stmt_execute(stmt) != 0)
    {
        fprintf(stderr, "Statement execution failed: %s\n", mysql_stmt_error(stmt));
        return std::nullopt;
    }

    // Obtener la cantidad de filas afectadas. Por lo general, para un UPDATE exitoso, debe ser al menos 1.
    my_ulonglong affected = mysql_stmt_affected_rows(stmt);
    if (affected == 0)
    {
        fprintf(stderr, "No rows updated. Verify that the address exists and belongs to the user.\n");
        return std::nullopt;
    }

    // En caso de éxito, retornamos la cantidad de filas actualizadas (usualmente 1).
    return static_cast<int>(affected);
} */

//---------------->>GET ADDRESS BY ID AND USER ID<<------------------//
 std::optional<Address> AddressModel::getAddressById(const int &address_id, const int &user_id, const std::string &type)
{
    // Obtener la conexión a la base de datos

    MYSQL *conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0)
    {
        std::cerr << "Error: No active database connection: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    Address address;

    const char *query = nullptr;

    if (type == "billing")
    {
        query =
            "SELECT id, first_name, last_name, phone, street, city, province, postal_code, country, is_default, additional_info "
            "FROM billing_addresses WHERE id = ? AND user_id = ?";
    }
    else
    {
        query =
            "SELECT id, first_name, last_name, phone, street, city, province, postal_code, country, is_default, additional_info "
            "FROM shipping_addresses WHERE id = ? AND user_id = ?";
    }
    // Inicializar la sentencia
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        std::cerr << "Statement initialization failed: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }
    // Utilizamos RAII para garantizar que se cierre el statement
    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);
    // Preparar la sentencia SQL
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0)
    {
        std::cerr << "Statement preparation failed: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }
    // Declarar el arreglo para los parámetros. La cantidad total de parámetros en la consulta es 2
    constexpr size_t NUM_PARAMS = 2;
    MYSQL_BIND param[NUM_PARAMS];
    memset(param, 0, sizeof(param));
    // Definir los parámetros en el mismo orden en el que aparecen en la consulta
    // 1. address_id
    param[0].buffer_type = MYSQL_TYPE_LONG;
    param[0].buffer = (void *)&address_id;
    param[0].buffer_length = sizeof(address_id);
    // 2. user_id
    param[1].buffer_type = MYSQL_TYPE_LONG;
    param[1].buffer = (void *)&user_id;
    param[1].buffer_length = sizeof(user_id);
    // Enlazar los parámetros a la sentencia
    if (mysql_stmt_bind_param(stmt, param) != 0)
    {
        std::cerr << "Parameter binding failed: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }
    // Ejecutar la sentencia
    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Statement execution failed: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }
    // Definir el arreglo para los resultados
    MYSQL_BIND result[11];
    memset(result, 0, sizeof(result));
    int id;
    char first_name[255], last_name[255], phone[50], street[255], city[100], province[100], postal_code[20], country[100], additional_info[1000];
    char is_default;

    unsigned long len[11];
    bool is_null[11];
    // id
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &id;
    result[0].is_null = &is_null[0];
    // first_name
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = first_name;
    result[1].buffer_length = sizeof(first_name);
    result[1].length = &len[1];
    result[1].is_null = &is_null[1];
    // last_name
    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = last_name;
    result[2].buffer_length = sizeof(last_name);
    result[2].length = &len[2];
    result[2].is_null = &is_null[2];
    // phone
    result[3].buffer_type = MYSQL_TYPE_STRING;
    result[3].buffer = phone;
    result[3].buffer_length = sizeof(phone);
    result[3].length = &len[3];
    result[3].is_null = &is_null[3];
    // street
    result[4].buffer_type = MYSQL_TYPE_STRING;
    result[4].buffer = street;
    result[4].buffer_length = sizeof(street);
    result[4].length = &len[4];
    result[4].is_null = &is_null[4];
    // city
    result[5].buffer_type = MYSQL_TYPE_STRING;
    result[5].buffer = city;
    result[5].buffer_length = sizeof(city);
    result[5].length = &len[5];
    result[5].is_null = &is_null[5];
    // province
    result[6].buffer_type = MYSQL_TYPE_STRING;
    result[6].buffer = province;
    result[6].buffer_length = sizeof(province);
    result[6].length = &len[6];
    result[6].is_null = &is_null[6];
    // postal_code
    result[7].buffer_type = MYSQL_TYPE_STRING;
    result[7].buffer = postal_code;
    result[7].buffer_length = sizeof(postal_code);
    result[7].length = &len[7];
    result[7].is_null = &is_null[7];
    // country
    result[8].buffer_type = MYSQL_TYPE_STRING;
    result[8].buffer = country;
    result[8].buffer_length = sizeof(country);
    result[8].length = &len[8];
    result[8].is_null = &is_null[8];
    // is_default
    result[9].buffer_type = MYSQL_TYPE_TINY;
    result[9].buffer = &is_default;
    result[9].is_null = &is_null[9];
    // addtional_info
    result[10].buffer_type = MYSQL_TYPE_STRING;
    result[10].buffer = additional_info;
    result[10].buffer_length = sizeof(additional_info);
    result[10].length = &len[10];
    result[10].is_null = &is_null[10];

    // Enlazar los resultados a la sentencia
    if (mysql_stmt_bind_result(stmt, result) != 0)
    {
        std::cerr << "Result binding failed: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }
    // Obtener los resultados
    int fetch_result = mysql_stmt_fetch(stmt);
    if (fetch_result == 0)
    {
        address.id = id;
        address.first_name = is_null[1] ? "" : std::string(first_name, len[1]);
        address.last_name = is_null[2] ? "" : std::string(last_name, len[2]);
        address.phone = is_null[3] ? "" : std::string(phone, len[3]);
        address.street = is_null[4] ? "" : std::string(street, len[4]);
        address.city = is_null[5] ? "" : std::string(city, len[5]);
        address.province = is_null[6] ? "" : std::string(province, len[6]);
        address.postal_code = is_null[7] ? "" : std::string(postal_code, len[7]);
        address.country = is_null[8] ? "" : std::string(country, len[8]);
        address.is_default = is_default;
        address.additional_info = is_null[10] ? "" : std::string(additional_info, len[10]);
    }
    else if (fetch_result == MYSQL_NO_DATA)
    {
        std::cerr << "⚠️ No se encontró la dirección con ID: " << address_id << " para el user_id: " << user_id << std::endl;
        return std::nullopt;
    }
    else
    {
        std::cerr << "❌ Error en mysql_stmt_fetch: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }
    std::cout << "Dirección encontrada: " << address.id << std::endl;
    return address;
} 