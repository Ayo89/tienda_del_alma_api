#include "model/ProductModel.h"
#include "db/DatabaseConnection.h"
#include <mysql/mysql.h>
#include "db/DatabaseInitializer.h"
#include <iostream> // Para manejo de errores o logs, si es necesario
#include <memory>   // Para std::unique_ptr
#include <cstring>  // Para strlen

std::optional<std::vector<Product>> ProductModel::getAllProducts()
{
    MYSQL *conn = db.getConnection();
    if (!conn || mysql_ping(conn) != 0)
    {
        std::cerr << "Error: No active database connection: " << mysql_error(conn) << std::endl;
        return std::nullopt;
    }

    std::vector<Product> products;
    const char *query = "SELECT id, sku, name, description, price, image_url, category_id FROM products";

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

    if (mysql_stmt_execute(stmt) != 0)
    {
        std::cerr << "Execution failed: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }

    MYSQL_BIND result[7];
    memset(result, 0, sizeof(result));
    int id, category_id;
    char sku[50], name[100], description[255], image_url[255];
    double price;
    unsigned long len[4];

    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &id;
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = sku;
    result[1].buffer_length = sizeof(sku);
    result[1].length = &len[1];
    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = name;
    result[2].buffer_length = sizeof(name);
    result[2].length = &len[2];
    result[3].buffer_type = MYSQL_TYPE_STRING;
    result[3].buffer = description;
    result[3].buffer_length = sizeof(description);
    result[3].length = &len[3];
    result[4].buffer_type = MYSQL_TYPE_DOUBLE;
    result[4].buffer = &price;
    result[5].buffer_type = MYSQL_TYPE_STRING;
    result[5].buffer = image_url;
    result[5].buffer_length = sizeof(image_url);
    result[5].length = &len[5];
    result[6].buffer_type = MYSQL_TYPE_LONG;
    result[6].buffer = &category_id;

    if (mysql_stmt_bind_result(stmt, result) != 0)
    {
        std::cerr << "Result binding failed: " << mysql_stmt_error(stmt) << std::endl;
        return std::nullopt;
    }

    while (mysql_stmt_fetch(stmt) == 0)
    {
        Product product;
        product.id = id;
        product.sku = std::string(sku, len[1]);
        product.name = std::string(name, len[2]);
        product.description = std::string(description, len[3]);
        product.price = price;
        product.image_url = std::string(image_url, len[5]);
        product.category_id = category_id;
        products.push_back(product);
    }

    return products;
}

bool ProductModel::insertSampleProducts()
{

    // INSERT CATEGORIES
    std::vector<std::string> categories = {
        "Incienso",
        "Velas",
        "Cristales",
        "Kits de Ritual"};
    std::vector<std::string> categoryQueries = {
        "INSERT INTO categories (name) VALUES "
        "('Incienso'),"
        "('Velas'),"
        "('Cristales'),"
        "('Kits de Ritual');"};
    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Error: No active database connection" << std::endl;
        return false;
    }
    DatabaseInitializer dbInitializer(db);
    for (const std::string &query : categoryQueries)
    {
        if (!dbInitializer.executeQuery(query))
        {
            std::cerr << "❌ Error al insertar categorías." << std::endl;
            return false;
        }
    }

    // Insert BRANDS
    std::vector<std::string> brandQueries = {
        "INSERT INTO brands (name) VALUES "
        "('LuzMística'),"
        "('Esencias del Alma'),"
        "('RitualZen');"};

    for (const std::string &query : brandQueries)
    {
        if (!dbInitializer.executeQuery(query))
        {
            std::cerr << "❌ Error al insertar marcas." << std::endl;
            return false;
        }
    }
    // INSERT PRODUCTS
    std::vector<std::string> queries = {
        "INSERT INTO products (sku, name, description, price, image_url, category_id, brand_id) VALUES "
        "('INC001', 'Incienso de Sándalo', 'Aroma calmante, ideal para meditación.', 4.50, 'https://example.com/images/incienso-sandalo.jpg', 1, 1),"
        "('INC002', 'Incienso de Lavanda', 'Relajante y purificante.', 4.00, 'https://example.com/images/incienso-lavanda.jpg', 1, 1),"
        "('INC003', 'Incienso de Mirra', 'Esotérico, ideal para rituales.', 4.75, 'https://example.com/images/incienso-mirra.jpg', 1, 2),"
        "('INC004', 'Incienso de Copal', 'Purificación energética profunda.', 5.00, 'https://example.com/images/incienso-copal.jpg', 1, 2),"
        "('INC005', 'Incienso de Rosa', 'Aroma dulce, usado en amor y armonía.', 4.25, 'https://example.com/images/incienso-rosa.jpg', 1, 3),"

        "('VEL001', 'Vela Blanca Ritual', 'Usada en rituales de limpieza y protección.', 3.50, 'https://example.com/images/vela-blanca.jpg', 2, 1),"
        "('VEL002', 'Vela Roja Ritual', 'Atrae amor y pasión.', 3.75, 'https://example.com/images/vela-roja.jpg', 2, 1),"
        "('VEL003', 'Vela Verde Prosperidad', 'Rituales de dinero y crecimiento.', 3.60, 'https://example.com/images/vela-verde.jpg', 2, 2),"
        "('VEL004', 'Vela Azul Paz', 'Ayuda a calmar el espíritu y dormir mejor.', 3.55, 'https://example.com/images/vela-azul.jpg', 2, 2),"
        "('VEL005', 'Vela Negra Protección', 'Rituales de protección intensa.', 3.90, 'https://example.com/images/vela-negra.jpg', 2, 3),"

        "('CRS001', 'Cuarzo Rosa', 'Cristal del amor y la armonía emocional.', 6.50, 'https://example.com/images/cuarzo-rosa.jpg', 3, 1),"
        "('CRS002', 'Amatista', 'Cristal para meditación y sueños lúcidos.', 6.75, 'https://example.com/images/amatista.jpg', 3, 1),"
        "('CRS003', 'Obsidiana Negra', 'Poderosa para protección energética.', 7.00, 'https://example.com/images/obsidiana.jpg', 3, 2),"
        "('CRS004', 'Citrino', 'Atrae éxito y energía positiva.', 6.90, 'https://example.com/images/citrino.jpg', 3, 2),"
        "('CRS005', 'Turmalina Negra', 'Escudo contra energías negativas.', 7.20, 'https://example.com/images/turmalina.jpg', 3, 3),"

        "('KIT001', 'Kit de Limpieza Energética', 'Incluye incienso, vela y cuarzo.', 12.00, 'https://example.com/images/kit-limpieza.jpg', 4, 1),"
        "('KIT002', 'Kit de Abundancia', 'Ritual para atraer prosperidad.', 13.50, 'https://example.com/images/kit-abundancia.jpg', 4, 2),"
        "('KIT003', 'Kit de Amor Propio', 'Fomenta autoestima y autocuidado.', 12.75, 'https://example.com/images/kit-amor.jpg', 4, 2),"
        "('KIT004', 'Kit de Protección Espiritual', 'Ideal para limpieza profunda.', 14.00, 'https://example.com/images/kit-proteccion.jpg', 4, 3),"
        "('KIT005', 'Kit Lunar Completo', 'Para rituales según las fases de la luna.', 15.50, 'https://example.com/images/kit-lunar.jpg', 4, 3);"};

    for (const std::string &query : queries)
    {
        if (!dbInitializer.executeQuery(query))
        {
            std::cerr << "❌ Error al insertar productos. MySql dice: " << mysql_error(conn) << "" << std::endl;
            return false;
        }
    }

    // Insertar inventario aleatorio entre 5 y 25 por producto
    std::string inventoryInsert =
        "INSERT INTO inventory (product_id, quantity) "
        "SELECT id, FLOOR(5 + RAND() * 21) FROM products "
        "WHERE sku LIKE 'INC%' OR sku LIKE 'VEL%' OR sku LIKE 'CRS%' OR sku LIKE 'KIT%';";

    if (!dbInitializer.executeQuery(inventoryInsert))
    {
        std::cerr << "❌ Error al insertar inventario." << std::endl;
        return false;
    }

    std::cout << "✅ Productos e inventario insertados correctamente." << std::endl;
    return true;
}
