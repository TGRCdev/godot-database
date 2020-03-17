#include "postgresql.h"

bool DatabasePostgreSQL::open(String args)
{
    ERR_FAIL_COND_V_MSG(_opened, false, "PostgreSQL database is already open!");

    this->connection = PQsetdbLogin("localhost", "5432", nullptr, nullptr, "postgres", "postgres", "pass");

    ConnStatusType status = PQstatus(this->connection);

    if(status == CONNECTION_BAD)
    {
        print_error("Failed to open Postgres database");
        PQfinish(this->connection);
        this->connection = nullptr;
        return false;
    }

    _opened = true;
    return _opened;
}

bool DatabasePostgreSQL::query(String statement)
{
    ERR_FAIL_COND_V_MSG(!_opened, false, "PostgreSQL database isn't open!");
    return true;
}

void DatabasePostgreSQL::close()
{
    ERR_FAIL_COND_MSG(!_opened, "PostgreSQL database isn't open!");

    PQfinish(this->connection);
    this->connection = nullptr;
    _opened = false;
}

String DatabasePostgreSQL::get_last_error()
{
    ERR_FAIL_COND_V_MSG(!_opened, "Database is not open", "PostgreSQL database isn't open!");

    return String(PQerrorMessage(this->connection));
}

void DatabasePostgreSQL::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("open", "args"), &DatabasePostgreSQL::open);
    ClassDB::bind_method(D_METHOD("get_last_error"), &DatabasePostgreSQL::get_last_error);
}