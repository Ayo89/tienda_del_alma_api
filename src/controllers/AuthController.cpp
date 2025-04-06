#include <cpprest/json.h>
#include <sodium.h>
#include <jwt-cpp/jwt.h>
#include "controllers/AuthController.h"
#include "db/DatabaseConnection.h"
#include "controllers/UserController.h"
#include "env/EnvLoader.h"
#include <cpprest/http_client.h>
#include "services/jwt/JwtService.h"
#include <future> // Para usar std::async

using namespace web;
using namespace web::http;
using namespace web::http::client;

http_response AuthController::signup(const http_request &request, DatabaseConnection &db)
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
                        auto first_name = utility::conversions::to_utf8string(body.at(U("first_name")).as_string());
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

                        // Llamar al controlador de usuarios
                        UserController userController(db);
                        bool success = userController.createUser(first_name, hashed, email);

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

                            auto token = JwtService::generateToken(first_name, email);
                            response.headers().add(U("X-Token"), utility::conversions::to_string_t(token));
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

http_response AuthController::login(const http_request &request, DatabaseConnection &db)
{
    // Respuesta predeterminada
    http_response response(status_codes::OK);
    EnvLoader env(".env");
    env.load(); // Cargar variables de entorno

    // Ejecutar la lógica de login de forma asíncrona
    std::future<void> login_future = std::async(std::launch::async, [request, &response, &db, &env]()
                                                {
                                                    // Extraer el cuerpo JSON del request
                                                    request.extract_json().then([&response, &db, &env](json::value body)
                                                                                {
            try {
                if (body.is_object())
                {
                    // Extraer email y password del JSON
                    auto email = utility::conversions::to_utf8string(body.at(U("email")).as_string());
                    std::string password = body.at(U("password")).as_string();

                    // Usamos el controlador de usuarios para buscar el usuario por email.
                    // Asegúrate de tener implementado getUserByEmail en UserController.
                    UserController userController(db);
                    auto userOpt = userController.getUserByEmail(email);
                    
                    if (!userOpt.has_value()) {
                        response.set_status_code(status_codes::Unauthorized);
                        response.set_body(json::value::object({
                            {U("error"), json::value::string(U("Usuario no encontrado"))}
                        }));
                        return;
                    }
                    
                    // Suponiendo que la entidad User tiene el campo password_hash
                    auto user = userOpt.value();

                    // Verificar la contraseña usando libsodium
                    if (crypto_pwhash_str_verify(user.password.c_str(), password.c_str(), password.size()) != 0)
                    {
                        response.set_status_code(status_codes::Unauthorized);
                        response.set_body(json::value::object({
                            {U("error"), json::value::string(U("Contraseña incorrecta"))}
                        }));
                        return;
                    }

                    // Si el login es exitoso, generar un token JWT
                    std::string secret = env.get("JWT_SECRET", "");
                    if (secret.empty())
                    {
                        response.set_status_code(status_codes::InternalError);
                        response.set_body(json::value::object({
                            {U("error"), json::value::string(U("JWT_SECRET no está configurado correctamente"))}
                        }));
                        return;
                    }

                    auto token = JwtService::generateToken(user.first_name, user.email);
                    response.headers().add(U("X-Token"), utility::conversions::to_string_t(token));
                    response.set_status_code(status_codes::OK);
                    response.set_body(json::value::object({
                        {U("message"), json::value::string(U("Login exitoso"))},
                        {U("token"), json::value::string(utility::conversions::to_string_t(token))}
                    }));
                }
                else
                {
                    response.set_status_code(status_codes::BadRequest);
                    response.set_body(json::value::object({
                        {U("error"), json::value::string(U("Cuerpo JSON inválido"))}
                    }));
                }
            }
            catch (const json::json_exception &e)
            {
                response.set_status_code(status_codes::BadRequest);
                response.set_body(json::value::object({
                    {U("error"), json::value::string(U("Error al procesar JSON: ") + utility::conversions::to_string_t(e.what()))}
                }));
            } })
                                                        .wait(); // Esperamos a que se procese el JSON dentro del lambda
                                                });

    // Esperamos a que se complete el futuro antes de retornar la respuesta
    login_future.wait();

    return response;
}