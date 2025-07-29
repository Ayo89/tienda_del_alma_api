
#include "controllers/AuthController.h"

using namespace web;
using namespace web::http;
using namespace web::http::client;

AuthController::AuthController() : userController() {}

http_response AuthController::signup(const http_request &request)
{
    // Creación de una respuesta predeterminada, que se completará al final del proceso
    http_response response(status_codes::OK);
    EnvLoader env(".env");
    env.load(); // Cargar el archivo .env

    // Este futuro representará la ejecución asincrónica de la lógica de signup
    std::future<void> signup_future = std::async(std::launch::async, [request, &response, &env, this]()
                                                 {
                                                     // Extraer el cuerpo JSON de la solicitud
                                                     request.extract_json()
                                                         .then([&response, &env, this](json::value body)
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
                        // Verificar si el usuario ya existe
                        auto userOpt = this->userController.getUserByEmail(email);
                         std::optional<int> user_id;
                        if (!userOpt.has_value())
                        {
                            // El usuario NO existe → crear nuevo usuario
                            user_id = this->userController.createUser(first_name, hashed, email, "local", "");  
                            std::cout<<user_id.value()<<std::endl;    
                        }else {}
                        // Llamar al controlador de usuarios
                        if (user_id.has_value()) {
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

                            auto token = JwtService::generateToken(std::to_string(user_id.value()), email);
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

http_response AuthController::login(const http_request &request)
{
    // Respuesta predeterminada
    http_response response(status_codes::OK);
    EnvLoader env(".env");
    env.load(); // Cargar variables de entorno

    // Ejecutar la lógica de login de forma asíncrona
    std::future<void> login_future = std::async(std::launch::async, [request, &response, &env, this]()
                                                {
                                                    // Extraer el cuerpo JSON del request
                                                    request.extract_json().then([&response, &env, this](json::value body)
                                                                                {
            try {
                if (body.is_object())
                {
                    // Extraer email y password del JSON
                    auto email = utility::conversions::to_utf8string(body.at(U("email")).as_string());
                    int user_id;
                    std::string password = body.at(U("password")).as_string();

                    // Usamos el controlador de usuarios para buscar el usuario por email.
                    // Asegúrate de tener implementado getUserByEmail en UserController.
                    auto userOpt = this->userController.getUserByEmail(email);
                    
                    if (!userOpt.has_value()) {
                        response.set_status_code(status_codes::Unauthorized);
                        response.set_body(json::value::object({
                            {U("error"), json::value::string(U("Usuario no encontrado"))}
                        }));
                        return;
                    }else {
                                    auto user = userOpt.value();

                                    if (user.auth_provider != "local") {
                                        // Conflicto: cuenta ya existe con otro método de login
                                        response.set_status_code(status_codes::Conflict); // 409
                                        response.set_body(json::value::object({
                                            {U("error"), json::value::string(U("Este email ya está registrado con otro método de autenticación. Inicia sesión con tu contraseña."))}
                                        }));
                                        return;
                                    }

                                

                                    user_id = user.id;
                                }
                    
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

                    auto token = JwtService::generateToken(std::to_string(user.id), user.email);
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

http_response AuthController::googleLogin(const http_request &request)
{
    http_response response(status_codes::OK);
    EnvLoader env(".env");
    env.load();

    std::cout << "📥 [googleLogin] Entrando..." << std::endl;

    try {
        auto body = request.extract_json().get();

        if (!body.has_field(U("id_token"))) {
            std::cerr << "❌ [googleLogin] No se encontró 'id_token' en el body" << std::endl;
            response.set_status_code(status_codes::BadRequest);
            response.set_body(json::value::object({ {U("error"), json::value::string(U("Falta el id_token"))} }));
            return response;
        }

        std::string id_token = utility::conversions::to_utf8string(body.at(U("id_token")).as_string());
        std::cout << "✅ [googleLogin] Token recibido" << std::endl;

        // Leer clave pública desde archivo
        std::string publicKeyPem = Auth0JwtUtils::readPemFile("config/auth0_public.pem");

        // Verificar y extraer claims
        auto decoded = Auth0JwtUtils::verifyAndExtractUser(
            id_token,
            publicKeyPem,
            env.get("GOOGLE_CLIENT_ID"),
            env.get("AUTH0_ISSUER")
        );

        std::string email = decoded.email;
        std::string sub = decoded.sub;
        std::string name = "Usuario Google";

        std::cout << "✅ [googleLogin] Token verificado correctamente. Email: " << email << " | Sub: " << sub << std::endl;

        // Buscar o crear usuario
        auto userOpt = this->userController.getUserByEmail(email);
        int user_id;

        if (!userOpt.has_value()) {
            std::cout << "👤 [googleLogin] Usuario no encontrado. Creando nuevo..." << std::endl;
            auto created = this->userController.createUser(name, "", email, "google", sub);
            if (!created.has_value()) {
                std::cerr << "❌ [googleLogin] Error al crear usuario" << std::endl;
                response.set_status_code(status_codes::InternalError);
                response.set_body(json::value::object({ {U("error"), json::value::string(U("Error al crear usuario"))} }));
                return response;
            }
            user_id = created.value();
        } else {
            auto user = userOpt.value();
            std::cout << "👤 [googleLogin] Usuario encontrado. Provider: " << user.auth_provider << std::endl;

            if (user.auth_provider != "google") {
                response.set_status_code(status_codes::Conflict);
                response.set_body(json::value::object({ {U("error"), json::value::string(U("Este email ya está registrado con otro método de autenticación. Inicia sesión con tu contraseña."))} }));
                return response;
            }

            if (!user.auth_id.empty() && user.auth_id != sub) {
                response.set_status_code(status_codes::Unauthorized);
                response.set_body(json::value::object({ {U("error"), json::value::string(U("El identificador de Google no coincide."))} }));
                return response;
            }

            user_id = user.id;
        }

        std::cout << "✅ [googleLogin] Usuario procesado. ID: " << user_id << std::endl;

        response.set_status_code(status_codes::OK);
        response.set_body(json::value::object({
            {U("message"), json::value::string(U("Login con Google exitoso"))},
            {U("success"), json::value::boolean(true)}
        }));
        return response;
    }
    catch (const std::exception &e)
    {
        std::cerr << "❌ [googleLogin] Excepción: " << e.what() << std::endl;
        response.set_status_code(status_codes::BadRequest);
        response.set_body(json::value::object({
            {U("error"), json::value::string(U(e.what()))}
        }));
        return response;
    }
}
