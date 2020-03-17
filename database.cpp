#include "database.h"

void Database::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("is_open"), &Database::is_open);
    ClassDB::bind_method(D_METHOD("query"), &Database::query);
    ClassDB::bind_method(D_METHOD("close"), &Database::close);
}