cmake_minimum_required(VERSION 3.10)
project(tienda_del_alma LANGUAGES CXX)

# 1️⃣ Haz que CMake genere compile_commands.json para el IDE
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 2️⃣ Opciones de estándar
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)

# 3️⃣ Trae jwt-cpp
FetchContent_Declare(
  jwt-cpp
  GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(jwt-cpp)

# 4️⃣ Dependencias del sistema
find_package(PkgConfig REQUIRED)
find_package(cpprestsdk REQUIRED)
find_package(OpenSSL REQUIRED)                # libssl
pkg_check_modules(MYSQL REQUIRED mysqlclient) # mysqlclient
pkg_check_modules(SODIUM REQUIRED libsodium)  # libsodium
find_library(UUID_LIBRARY NAMES uuid) # uuid

# 5️⃣ Define tu ejecutable y sus fuentes
add_executable(tienda_del_alma
  main.cpp 
  src/db/DatabaseConnection.cpp
  src/db/DatabaseInitializer.cpp
  src/env/EnvLoader.cpp
  src/server/Server.cpp
  src/services/jwt/JwtService.cpp
  src/router/Router.cpp
  src/controllers/AuthController.cpp
  src/controllers/AuthUtils.cpp
  src/controllers/UserController.cpp
  src/controllers/ProductController.cpp
  src/controllers/AddressController.cpp
  src/controllers/OrderController.cpp
  src/controllers/PaypalController.cpp
  src/controllers/CarrierController.cpp
  src/model/UserModel.cpp
  src/model/ProductModel.cpp
  src/model/AddressModel.cpp
  src/model/OrderModel.cpp
  src/model/OrderItemModel.cpp
  src/model/CarrierModel.cpp
  src/utils/UtilsOwner.cpp
  src/services/PaypalService.cpp
)

# 6️⃣ Rutas de inclusión PER-TARGET (mejor que include_directories)
target_include_directories(tienda_del_alma PRIVATE
  ${MYSQL_INCLUDE_DIRS}
  ${SODIUM_INCLUDE_DIRS}
  ${OPENSSL_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/include   # si tienes headers en include/
)

# 7️⃣ Linkea las librerías necesarias
target_link_libraries(tienda_del_alma PRIVATE
  cpprestsdk::cpprest
  OpenSSL::SSL
  OpenSSL::Crypto
  ${MYSQL_LIBRARIES}
  ${SODIUM_LIBRARIES}
  jwt-cpp
  ${UUID_LIBRARY}
)

# 8️⃣ Asegúrate de exportar también los flags de compilación a tu IDE
#    — en VS Code pon en .vscode/c_cpp_properties.json:
#      "compileCommands": "${workspaceFolder}/build/compile_commands.json"
