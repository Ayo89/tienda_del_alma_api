#include <sodium.h>
#include "db/DatabaseConnection.h"
#include "controllers/AuthController.h"
#include "server/Server.h"
#include "env/EnvLoader.h"
#include <iostream>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include "db/DatabaseInitializer.h"
#include "model/ProductModel.h"
#include "model/CarrierModel.h"
#include <mutex>
#include <condition_variable>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace std;

int main()
{
    EnvLoader env(".env");
    env.load();

    if (sodium_init() < 0)
    {
        std::cerr << "Error inicializando libsodium" << std::endl;
        return 1;
    }

    string host = env.get("DB_HOST", "localhost");
    string user = env.get("DB_USER", "root");
    string password = env.get("DB_PASSWORD", "");
    string dbname = env.get("DB_NAME", "tienda_del_alma");
    unsigned int dbPort = stoi(env.get("DB_PORT", "3306"));

    std::string appPort = env.get("PORT", "8080");
    std::string serverAddressStr = "http://0.0.0.0:" + appPort;
    utility::string_t server_address = utility::conversions::to_string_t(serverAddressStr);

    DatabaseInitializer dbInitializer;
    if (!dbInitializer.initialize(true))
    {
        std::cerr << "Error al inicializar la base de datos." << std::endl;
        return 1;
    }

    std::cout << "Base de datos inicializada correctamente." << std::endl;

    ProductModel productModel;
    if (!productModel.insertSampleProducts())
    {
        wcout << L"Error: No se pudo insertar productos de muestra" << endl;
    }

    CarrierModel carrierModel;
    if (!carrierModel.insertSampleCarriers())
    {
        wcout << L"Error: No se pudo insertar transportistas de muestra" << endl;
    }

    try
    {
        Server server(server_address);
        server.start();

        std::mutex mtx;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock);

        server.stop();
    }
    catch (const exception &err)
    {
        cerr << "Error: " << err.what() << endl;
        return 1;
    }

    return 0;
}