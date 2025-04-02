#ifndef ENV_LOADER_H
#define ENV_LOADER_H

#include <map>
#include <string>

class EnvLoader
{
public:
    // Constructor que opcionalmente toma el nombre del archivo .env
    EnvLoader(const std::string &filename = ".env");

    // Método para cargar las variables del archivo
    void load();

    // Método para obtener una variable con un valor por defecto
    std::string get(const std::string &key, const std::string &default_value = "") const;

    // Método para verificar si una clave existe
    bool has(const std::string &key) const;

private:
    std::map<std::string, std::string> env_; // Almacena las variables cargadas
    std::string filename_;                   // Nombre del archivo .env
};

#endif // ENV_LOADER_H