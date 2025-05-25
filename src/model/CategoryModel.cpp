#include "model/CategoryModel.h"

std::pair<std::optional<std::vector<Category>>, Errors> CategoryModel::getAllCategories()
{

    DatabaseConnection &db = DatabaseConnection::getInstance();

    MYSQL *connection = db.getConnection();

    if (!connection)
    {
        std::cerr << "Database connection failed." << std::endl;
        return {std::nullopt, Errors::DatabaseConnectionFailed};
    }

    const char *query = "SELECT id, name, description, created_at FROM categories";

    MYSQL_STMT *stmt = mysql_stmt_init(connection);

    if (!stmt)
    {
        std::cerr << "Failed to initialize statement: " << mysql_error(connection) << std::endl;
        return {std::nullopt, Errors::StatementInitFailed};
    }

    auto stmt_guard = std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>(stmt, mysql_stmt_close);

    if (mysql_stmt_prepare(stmt, query, strlen(query)))
    {
        std::cerr << "Failed to prepare statement: " << mysql_error(connection) << std::endl;
        return {std::nullopt, Errors::StatementPrepareFailed};
    }

    if (mysql_stmt_execute(stmt))
    {
        std::cerr << "Failed to execute statement: " << mysql_error(connection) << std::endl;
        return {std::nullopt, Errors::ExecutionFailed};
    }

    if (mysql_stmt_store_result(stmt))
    {
        std::cerr << "Failed to store result: " << mysql_error(connection) << std::endl;
        return {std::nullopt, Errors::StoreResultFailed};
    }

    MYSQL_BIND result_bind[4];
    memset(result_bind, 0, sizeof(result_bind));
    int id;
    char name[100];
    char description[255];
    char created_at[20];
    bool is_null[4]{};

    result_bind[0].buffer_type = MYSQL_TYPE_LONG;
    result_bind[0].buffer = &id;
    result_bind[0].buffer_length = sizeof(id);
    result_bind[0].is_null = &is_null[0];

    result_bind[1].buffer_type = MYSQL_TYPE_STRING;
    result_bind[1].buffer = name;
    result_bind[1].buffer_length = sizeof(name);
    result_bind[1].is_null = &is_null[1];

    result_bind[2].buffer_type = MYSQL_TYPE_STRING;
    result_bind[2].buffer = description;
    result_bind[2].buffer_length = sizeof(description);
    result_bind[2].is_null = &is_null[2];

    result_bind[3].buffer_type = MYSQL_TYPE_STRING;
    result_bind[3].buffer = created_at;
    result_bind[3].buffer_length = sizeof(created_at);
    result_bind[3].is_null = &is_null[3];

    if (mysql_stmt_bind_result(stmt, result_bind))
    {
        std::cerr << "Failed to bind result: " << mysql_error(connection) << std::endl;
        return {std::nullopt, Errors::BindResultFailed};
    }

    std::vector<Category> categories;
    while (mysql_stmt_fetch(stmt) == 0)
    {
        if (!is_null[0])
        {
            Category category;
            category.id = id;
            category.name = is_null[1] ? "" : std::string(name);
            category.description = is_null[2] ? "" : std::string(description);
            category.created_at = is_null[3] ? "" : std::string(created_at);
            categories.push_back(category);
        }
    }

    return {categories, Errors::NoError};
}
