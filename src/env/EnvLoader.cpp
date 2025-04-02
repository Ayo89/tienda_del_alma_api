#include "env/EnvLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>

EnvLoader::EnvLoader(const std::string &filename) : filename_(filename)
{
    // No cargamos aquí, dejamos que el usuario llame a load() explícitamente
}

void EnvLoader::load()
{
    std::ifstream file(filename_);
    if (!file.is_open())
    {
        std::cerr << "No se pudo abrir el archivo " << filename_ << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
        {
            continue; // Ignorar líneas vacías o comentarios
        }
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value))
        {
            env_[key] = value;
        }
    }
    file.close();
}

std::string EnvLoader::get(const std::string &key, const std::string &default_value) const
{
    auto it = env_.find(key);
    if (it != env_.end())
    {
        return it->second;
    }
    return default_value;
}

bool EnvLoader::has(const std::string &key) const
{
    return env_.find(key) != env_.end();
}