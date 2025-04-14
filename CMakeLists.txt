cmake_minimum_required(VERSION 3.10)

project(tienda_del_alma)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  jwt-cpp
  GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
  GIT_TAG        master  # Puedes especificar una versión o commit concreto
)

FetchContent_MakeAvailable(jwt-cpp)


# Encuentra las dependencias
find_package(PkgConfig REQUIRED)
find_package(cpprestsdk REQUIRED)
pkg_check_modules(MYSQL REQUIRED mysqlclient)
find_package(OpenSSL REQUIRED)  # Añadido para OpenSSL
pkg_check_modules(SODIUM REQUIRED libsodium)  # Añadido para libsodium

# Incluir directorios de encabezados
include_directories(${MYSQL_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/include ${OPENSSL_INCLUDE_DIR} ${SODIUM_INCLUDE_DIRS})

# Agregar archivos fuente
add_executable(tienda_del_alma 
    main.cpp 
    src/db/DatabaseConnection.cpp
    src/db/DatabaseInitializer.cpp
    src/server/Server.cpp
    src/env/EnvLoader.cpp
    src/services/jwt/JwtService.cpp
    src/router/Router.cpp
    src/controllers/AuthController.cpp
    src/controllers/UserController.cpp
    src/controllers/ProductController.cpp
    src/controllers/AddressController.cpp
    src/controllers/AuthUtils.cpp
    src/model/UserModel.cpp
    src/model/ProductModel.cpp
    src/model/AddressModel.cpp

)

# Vincular bibliotecas
target_link_libraries(tienda_del_alma 
    PRIVATE 
    ${MYSQL_LIBRARIES}
    cpprestsdk::cpprest
    OpenSSL::SSL    # Vincula libssl
    OpenSSL::Crypto # Vincula libcrypto
    ${SODIUM_LIBRARIES}  # Vincula libsodium
    jwt-cpp
)
