#ifndef GODOT_POSTGRESQL_H
#define GODOT_POSTGRESQL_H

#include "database.h"
#include "postgresql/libpq-fe.h"

class DatabasePostgreSQL : public Database
{
    GDCLASS(DatabasePostgreSQL, Database);

    protected:
    static void _bind_methods();
    PGconn *connection;

    public:

    DatabasePostgreSQL()
    {
        _opened = false;
        connection = nullptr;
    }

    bool open(String args);

    String get_last_error();

    virtual bool query(String statement);
    virtual void close();
};

#endif