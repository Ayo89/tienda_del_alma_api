#include "model/ProductModel.h"
#include "db/DatabaseConnection.h"
#include <mysql/mysql.h>
#include "db/DatabaseInitializer.h"
#include <iostream> // Para manejo de errores o logs, si es necesario

std::vector<Product> ProductModel::getAllProducts()
{
    std::vector<Product> products;
    std::string query = "SELECT id, sku, name, description, price, image_url, category_id FROM products;";

    MYSQL *conn = db.getConnection();
    if (!conn)
    {
        std::cerr << "Error: No active database connection" << std::endl;
        return products;
    }

    if (mysql_query(conn, query.c_str()))
    {
        std::cerr << "Error al ejecutar la consulta: " << mysql_error(conn) << std::endl;
        return products;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (!result)
    {
        std::cerr << "Error al obtener resultados: " << mysql_error(conn) << std::endl;
        return products;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)))
    {
        try
        {
            int id = std::stoi(row[0]);
            std::string sku = row[1] ? row[1] : "";
            std::string name = row[2] ? row[2] : "";
            std::string description = row[3] ? row[3] : "";
            double price = row[4] ? std::stod(row[4]) : 0.0;
            std::string imageUrl = row[5] ? row[5] : "";
            int categoryId = row[6] ? std::stoi(row[6]) : 0;

            Product product(id, sku, name, description, price, imageUrl, categoryId);
            products.push_back(product);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error al convertir datos: " << e.what() << std::endl;
            continue; // Ignora el registro problemático
        }
    }

    mysql_free_result(result);
    return products;
}

bool ProductModel::insertSampleProducts()
{
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

    std::vector<std::string> queries = {
        "INSERT INTO products (sku, name, description, price, image_url, category_id) VALUES "
        "('INC001', 'Incienso de Sándalo', 'Aroma calmante, ideal para meditación.', 4.50, 'https://example.com/images/incienso-sandalo.jpg', 1),"
        "('INC002', 'Incienso de Lavanda', 'Relajante y purificante.', 4.00, 'https://example.com/images/incienso-lavanda.jpg', 1),"
        "('INC003', 'Incienso de Mirra', 'Esotérico, ideal para rituales.', 4.75, 'https://example.com/images/incienso-mirra.jpg', 1),"
        "('INC004', 'Incienso de Copal', 'Purificación energética profunda.', 5.00, 'https://example.com/images/incienso-copal.jpg', 1),"
        "('INC005', 'Incienso de Rosa', 'Aroma dulce, usado en amor y armonía.', 4.25, 'https://example.com/images/incienso-rosa.jpg', 1),"

        "('VEL001', 'Vela Blanca Ritual', 'Usada en rituales de limpieza y protección.', 3.50, 'https://example.com/images/vela-blanca.jpg', 2),"
        "('VEL002', 'Vela Roja Ritual', 'Atrae amor y pasión.', 3.75, 'https://example.com/images/vela-roja.jpg', 2),"
        "('VEL003', 'Vela Verde Prosperidad', 'Rituales de dinero y crecimiento.', 3.60, 'https://example.com/images/vela-verde.jpg', 2),"
        "('VEL004', 'Vela Azul Paz', 'Ayuda a calmar el espíritu y dormir mejor.', 3.55, 'https://example.com/images/vela-azul.jpg', 2),"
        "('VEL005', 'Vela Negra Protección', 'Rituales de protección intensa.', 3.90, 'https://example.com/images/vela-negra.jpg', 2),"

        "('CRS001', 'Cuarzo Rosa', 'Cristal del amor y la armonía emocional.', 6.50, 'https://example.com/images/cuarzo-rosa.jpg', 3),"
        "('CRS002', 'Amatista', 'Cristal para meditación y sueños lúcidos.', 6.75, 'https://example.com/images/amatista.jpg', 3),"
        "('CRS003', 'Obsidiana Negra', 'Poderosa para protección energética.', 7.00, 'https://example.com/images/obsidiana.jpg', 3),"
        "('CRS004', 'Citrino', 'Atrae éxito y energía positiva.', 6.90, 'https://example.com/images/citrino.jpg', 3),"
        "('CRS005', 'Turmalina Negra', 'Escudo contra energías negativas.', 7.20, 'https://example.com/images/turmalina.jpg', 3),"

        "('KIT001', 'Kit de Limpieza Energética', 'Incluye incienso, vela y cuarzo.', 12.00, 'https://example.com/images/kit-limpieza.jpg', 4),"
        "('KIT002', 'Kit de Abundancia', 'Ritual para atraer prosperidad.', 13.50, 'https://example.com/images/kit-abundancia.jpg', 4),"
        "('KIT003', 'Kit de Amor Propio', 'Fomenta autoestima y autocuidado.', 12.75, 'https://example.com/images/kit-amor.jpg', 4),"
        "('KIT004', 'Kit de Protección Espiritual', 'Ideal para limpieza profunda.', 14.00, 'https://example.com/images/kit-proteccion.jpg', 4),"
        "('KIT005', 'Kit Lunar Completo', 'Para rituales según las fases de la luna.', 15.50, 'https://example.com/images/kit-lunar.jpg', 4);"};



    for (const std::string &query : queries)
    {
        if (!dbInitializer.executeQuery(query))
        {
            std::cerr << "❌ Error al insertar productos." << std::endl;
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
