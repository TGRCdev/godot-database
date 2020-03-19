#ifndef GODOT_DATABASE_SQLITE_H
#define GODOT_DATABASE_SQLITE_H

#include "database.h"
#include "cursor.h"
#include "../thirdparty/sqlite/sqlite3.h"

class CursorSQLite;

class DatabaseSQLite : public Database
{
    friend class CursorSQLite;
    GDCLASS(DatabaseSQLite, Database);

    protected:
    static void _bind_methods();

    String filepath;

    sqlite3 *connection = nullptr;

    bool auto_commit;
    bool in_transaction; // True if there was a transaction started by the SQLite wrapper

    /// Called after every commit() and rollback() when
    /// auto-commit is disabled
    void begin_transaction();

    /// Prepares and executes the statement
    /// Returns false if an error occurred, true
    /// otherwise.
    bool exec_statement(const char *statement);

    sqlite3_stmt *prepare_statement(const char *statement);

    public:

    virtual bool is_open();

    virtual void close();

    virtual void commit();

    virtual void rollback();

    /// Enable or disable auto-commit transactions
    /// False by default
    void set_auto_commit(bool value);
    bool get_auto_commit() const {return auto_commit;}

    bool open(String path, int flags);

    String get_filepath() const {return filepath;}

    virtual Ref<Cursor> cursor();
};

class CursorSQLite : public Cursor
{
    friend class DatabaseSQLite;
    GDCLASS(CursorSQLite, Cursor);

    protected:
    static void _bind_methods() {};

    Dictionary parse_row(sqlite3_stmt *stmt);

    /// Binds the parameters for a statement
    /// Returns false if an error occurs while binding
    bool bind_parameters(sqlite3_stmt *stmt, Array arguments);

    Array last_result;
    int result_pos;

    bool _open;

    Ref<DatabaseSQLite> database;

    virtual bool is_open() {return database.is_valid() && database->is_open() && _open;}
    virtual void close() {database.unref(); _open = false;}

    virtual bool callproc(String procname, Array arguments);
    virtual bool execute(String statement, Array arguments);
    virtual bool execute_many(String statement, Array arguments);

    virtual int get_row_count();

    virtual void scroll(int amount, bool absolute);

    virtual Dictionary fetch_one();
    virtual Array fetch_many(int size);
    virtual Array fetch_all();
};

#endif