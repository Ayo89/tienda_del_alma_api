#include <sodium.h>
#include "db/DatabaseConnection.h"
#include "controllers/AuthController.h"
#include "server/Server.h"
#include "env/EnvLoader.h"
#include <iostream>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <mutex>
#include <condition_variable>
#include "jobs/InventoryExpirationJob.h" 


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

    std::string appPort = env.get("PORT", "8080");
    std::string serverAddressStr = "http://0.0.0.0:" + appPort;
    utility::string_t server_address = utility::conversions::to_string_t(serverAddressStr);

    try
    {
        Server server(server_address);
        server.start();

        startInventoryExpirationJob(); 
        
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