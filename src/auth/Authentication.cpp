#include <cpprest/json.h>
#include <sodium.h>
#include <jwt-cpp/jwt.h>
#include "auth/Authentication.h"
#include "db/DatabaseConnection.h"
#include "model/UserModel.h"
#include "env/EnvLoader.h"
#include <cpprest/http_client.h>
#include <future> // Para usar std::async

using namespace web;
using namespace web::http;
using namespace web::http::client;

http_response Authentication::signup(const http_request &request, DatabaseConnection &db)
{
    // Creación de una respuesta predeterminada, que se completará al final del proceso
    http_response response(status_codes::OK);
    EnvLoader env(".env");
    env.load(); // Cargar el archivo .env

    // Este futuro representará la ejecución asincrónica de la lógica de signup
    std::future<void> signup_future = std::async(std::launch::async, [request, &response, &db, &env]()
                                                 {
                                                     // Extraer el cuerpo JSON de la solicitud
                                                     request.extract_json()
                                                         .then([&response, &db, &env](json::value body)
                                                               {
                try {
                    // Verificar que el cuerpo sea un objeto JSON válido
                    if (body.is_object()) {
                        // Extraer los campos del JSON
                        auto name = utility::conversions::to_utf8string(body.at(U("name")).as_string());
                        std::string password = body.at(U("password")).as_string();
                        auto email = utility::conversions::to_utf8string(body.at(U("email")).as_string());

                        char hashed[crypto_pwhash_STRBYTES];

                        if (crypto_pwhash_str(hashed, password.c_str(), password.size(),
                                              crypto_pwhash_OPSLIMIT_MODERATE,
                                              crypto_pwhash_MEMLIMIT_MODERATE) != 0)
                        {
                            std::cerr << "Error al generar el hash" << std::endl;
                            response.set_status_code(status_codes::InternalError);
                            response.set_body(json::value::object({{U("error"), json::value::string(U("Error al generar el hash de la contraseña"))}}));
                            return; // exit lambda
                        }

                        // Llamar a UserModel::createUser
                        bool success = UserModel::createUser(name, hashed, email, db);

                        if (success) {
                            std::string secret = env.get("JWT_SECRET", "");
                            if (secret.empty())
                            {
                                std::cerr << "Error: JWT_SECRET no está configurado correctamente." << std::endl;
                                response.set_status_code(status_codes::InternalError);
                                response.set_body(json::value::object({
                                    {U("error"), json::value::string(U("JWT_SECRET no está configurado correctamente"))}
                                }));
                                return;
                            }

                            auto token = jwt::create()
                                             .set_issuer("tienda_del_alma")
                                             .set_payload_claim("name", jwt::claim(name))
                                             .set_payload_claim("email", jwt::claim(email))
                                             .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(24))
                                             .sign(jwt::algorithm::hs256{secret});

                            response.set_status_code(status_codes::Created); // 201 Created
                            response.set_body(json::value::object({{U("message"), json::value::string(U("Usuario creado exitosamente"))},
                                                                   {U("token"), json::value::string(utility::conversions::to_string_t(token))}}));
                        } else {
                            response.set_status_code(status_codes::InternalError); // 500
                            response.set_body(json::value::object({
                                {U("error"), json::value::string(U("Error al crear usuario"))}
                            }));
                        }
                    } else {
                        response.set_status_code(status_codes::BadRequest); // 400
                        response.set_body(json::value::object({
                            {U("error"), json::value::string(U("Cuerpo JSON inválido"))}
                        }));
                    }
                } catch (const json::json_exception &e) {
                    response.set_status_code(status_codes::BadRequest); // 400
                    response.set_body(json::value::object({
                        {U("error"), json::value::string(U("Error al procesar JSON: ") + utility::conversions::to_string_t(e.what()))}
                    }));
                } })
                                                         .wait(); // Usamos `wait()` solo dentro de la ejecución asincrónica, no bloqueando el hilo principal
                                                 });

    // Esperamos a que se complete el futuro antes de retornar
    signup_future.wait(); // Bloquea solo la parte del código que necesita esperar el resultado

    return response; // Esto se ejecuta cuando el futuro ha completado su trabajo
}
