#ifndef CATEGORYMODEL_H
#define CATEGORYMODEL_H
#include "db/DatabaseConnection.h"
#include "entities/Category.h"
#include <optional>
#include <vector>
#include "utils/Errors.h"
#include <memory>
#include <string>
#include <iostream>
#include <cstring>

class CategoryModel
{
private:
public:
    CategoryModel() {};
    std::pair<std::optional<std::vector<Category>>, Errors> getAllCategories();
};

#endif
