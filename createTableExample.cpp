/*     // Crear la tabla 'productos' si no existe
      const char *crearTabla = "CREATE TABLE IF NOT EXISTS productos ("
                               "id INT AUTO_INCREMENT PRIMARY KEY, "
                               "nombre VARCHAR(255) NOT NULL, "
                               "precio DECIMAL(10, 2) NOT NULL, "
                               "stock INT NOT NULL);";

      if (mysql_query(conexion, crearTabla))
      {
          std::cerr << "Error al crear la tabla: " << mysql_error(conexion) << std::endl;
          mysql_close(conexion);
          return;
      }

      std::cout << "Tabla 'productos' creada con éxito." << std::endl;

      // Ejecuta una consulta
      const char *consulta = "SELECT * FROM productos;";
      if (mysql_query(conexion, consulta))
      {
          std::cerr << "Error al ejecutar consulta: " << mysql_error(conexion) << std::endl;
          mysql_close(conexion);
          return;
      }

      // Obtén los resultados
      MYSQL_RES *resultado = mysql_store_result(conexion);
      if (resultado == nullptr)
      {
          std::cerr << "Error al obtener resultados: " << mysql_error(conexion) << std::endl;
          mysql_close(conexion);
          return;
      }

      // Muestra los resultados
      MYSQL_ROW fila;
      while ((fila = mysql_fetch_row(resultado)))
      {
          std::cout << "ID: " << fila[0] << ", Nombre: " << fila[1] << ", Precio: " << fila[2] << ", Stock: " << fila[3] << std::endl;
      } */

// Limpia recursos
/*     mysql_free_result(resultado); */