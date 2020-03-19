#include "database.h"

void Database::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("is_open"), &Database::is_open);
    ClassDB::bind_method(D_METHOD("close"), &Database::close);
    ClassDB::bind_method(D_METHOD("commit"), &Database::commit);
    ClassDB::bind_method(D_METHOD("rollback"), &Database::rollback);
    ClassDB::bind_method(D_METHOD("cursor"), &Database::cursor);
}