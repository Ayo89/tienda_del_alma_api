#pragma once

#include <string>


struct DecodedUser {
    int id;                  // ID del usuario
    std::string sub;            // ID único de Google
    std::string email;
    std::string issuer;        // emisor del token (ej. "accounts.google.com")
    bool email_verified = false;
    std::string name;           // nombre completo
    std::string given_name;     // nombre (de pila)
    std::string family_name;    // apellido
    std::string picture;        // URL de la foto de perfil
    std::string locale;         // idioma del perfil (ej. "es")
};