// src/main.cpp

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

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace std;

int main()
{

    EnvLoader env(".env");
    env.load();

    // Load hashed function
    if (sodium_init() < 0)
    {
        std::cerr << "Error inicializando libsodium" << std::endl;
        return 1;
    }

    // Leer las variables
    string host = env.get("DB_HOST", "localhost"); // Valor por defecto si no está en .env
    string user = env.get("DB_USER", "root");
    string password = env.get("DB_PASSWORD", "");
    string dbname = env.get("DB_NAME", "tienda_del_alma");
    unsigned int port = stoi(env.get("DB_PORT", "3306"));
    utility::string_t server_address = U(env.get("SERVER_ADDRESS", "http://localhost:8080"));

    DatabaseInitializer dbInitializer;
    if (!dbInitializer.initialize(true))
    {
        std::cerr << "Error al inicializar la base de datos." << std::endl;
        return 1;
    }
    else
    {
        std::cout << "Base de datos inicializada correctamente." << std::endl;
    }

    ProductModel productModel;
    if (!productModel.insertSampleProducts())
    {
        wcout << L"Error: No se pudo insertar productos de muestra" << endl;
    }
    CarrierModel carrierModel;
    if (!carrierModel.insertSampleCarriers()){
        wcout << L"Error: No se pudo insertar transportistas de muestra" << endl;
    }

    try
    {
        Server server(server_address);
        server.start();
        string line;
        getline(cin, line);
        server.stop();
    }
    catch (const exception &err)
    {
        wcout << L"Error: " << err.what() << endl;
        return 1;
    }

    return 0;
}
