#include "register_types.h"

#include "core/class_db.h"

#include "src/database.h"
#include "src/cursor.h"
#include "src/db_sqlite.h"

void register_database_types()
{
    ClassDB::register_virtual_class<Database>();
    ClassDB::register_virtual_class<Cursor>();
    sqlite3_initialize();
    ClassDB::register_class<DatabaseSQLite>();
    ClassDB::register_class<CursorSQLite>();
}

void unregister_database_types()
{
    sqlite3_shutdown();
}