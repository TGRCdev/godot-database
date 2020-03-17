#include "register_types.h"

#include "core/class_db.h"

#include "database.h"
#include "postgresql.h"

void register_database_types()
{
    ClassDB::register_virtual_class<Database>();
    ClassDB::register_class<DatabasePostgreSQL>();
}

void unregister_database_types()
{
    
}