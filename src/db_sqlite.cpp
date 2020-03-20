#include "db_sqlite.h"
#include "core/os/os.h"
#include "editor/project_settings_editor.h"

void DatabaseSQLite::_bind_methods()
{
    BIND_ENUM_CONSTANT(OPEN_READONLY);
    BIND_ENUM_CONSTANT(OPEN_READWRITE);
    BIND_ENUM_CONSTANT(OPEN_CREATE);
    BIND_ENUM_CONSTANT(OPEN_MEMORY);
    BIND_ENUM_CONSTANT(OPEN_URI);
    BIND_ENUM_CONSTANT(OPEN_NOFOLLOW);

    ClassDB::bind_method(D_METHOD("open", "path", "flags"), &DatabaseSQLite::open);
    ClassDB::bind_method(D_METHOD("get_filepath"), &DatabaseSQLite::get_filepath);
    ClassDB::bind_method(D_METHOD("set_auto_commit", "value"), &DatabaseSQLite::set_auto_commit);
    ClassDB::bind_method(D_METHOD("get_auto_commit"), &DatabaseSQLite::get_auto_commit);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_commit"), "set_auto_commit", "get_auto_commit");
}

bool DatabaseSQLite::is_open()
{
    return connection != nullptr;
}

void DatabaseSQLite::close()
{
    ERR_FAIL_COND_MSG(!is_open(), "SQLite database is not open!");

    sqlite3_close_v2(connection);
    connection = nullptr;
}

sqlite3_stmt *DatabaseSQLite::prepare_statement(const char *statement)
{
    sqlite3_stmt* stmt;
    int err = sqlite3_prepare_v3(connection, statement, -1, 0, &stmt, nullptr);
    if(err != SQLITE_OK)
    {
        print_error(String("SQLite error: ") + sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        return nullptr;
    }

    return stmt;
}

bool DatabaseSQLite::exec_statement(const char *statement)
{
    sqlite3_stmt* stmt = prepare_statement(statement);

    if(stmt == nullptr)
    {
        return false;
    }

    int err;
    do
    {
        err = sqlite3_step(stmt);
    }
    while(err == SQLITE_BUSY);

    if(err != SQLITE_DONE)
    {
        print_error(String("SQLite error: ") + sqlite3_errmsg(connection));
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

void DatabaseSQLite::begin_transaction()
{
    bool success = exec_statement("BEGIN TRANSACTION");
    ERR_FAIL_COND_MSG(!success, "SQLite failed to begin a new transaction");

    in_transaction = true;
}

void DatabaseSQLite::commit()
{
    ERR_FAIL_COND_MSG(!is_open(), "SQLite database is not open!");

    bool success = exec_statement("END TRANSACTION");
    ERR_FAIL_COND_MSG(!success, "SQLite failed to end a transaction");

    in_transaction = false;
    if(!auto_commit)
        begin_transaction();
}

void DatabaseSQLite::rollback()
{
    ERR_FAIL_COND_MSG(!is_open(), "SQLite database is not open!");

    bool success = exec_statement("ROLLBACK TRANSACTION");
    ERR_FAIL_COND_MSG(!success, "SQLite failed to rollback a transaction");

    in_transaction = false;
    if(!auto_commit)
        begin_transaction();
}

void DatabaseSQLite::set_auto_commit(bool value)
{
    auto_commit = value;

    if(connection != nullptr)
    {
        if(!auto_commit && !in_transaction)
        {
            begin_transaction();
        }
        else if(auto_commit && in_transaction)
        {
            commit();
        }
    }
}

bool DatabaseSQLite::open(String path, int flags)
{
    ERR_FAIL_COND_V_MSG(is_open(), false, "SQLite database is already open!");

    path = path.strip_edges();

    if(path.empty())
        return false;
    
    // TODO: Packed res:// databases

    if(!path.begins_with(":") && (flags & SQLITE_OPEN_MEMORY) == 0)
    {
        path = ProjectSettings::get_singleton()->globalize_path(path);
    }

    int err = sqlite3_open_v2(path.utf8().get_data(), &connection, flags, nullptr);

    if(err != SQLITE_OK)
    {
        print_error(String("An error occurred while opening an SQLite database: ") + sqlite3_errstr(err));
        connection = nullptr;
        return false;
    }

    filepath = path;

    if(!auto_commit)
        begin_transaction();

    return true;
}

Ref<Cursor> DatabaseSQLite::cursor()
{
    ERR_FAIL_COND_V_MSG(!is_open(), Ref<CursorSQLite>(), "SQLite database is not open!");

    Ref<CursorSQLite> new_cursor;
    new_cursor.instance();
    new_cursor->database = Ref(this);

    return new_cursor;
}

bool CursorSQLite::callproc(String procname, Array arguments)
{
    ERR_FAIL_V_MSG(false, "SQLite does not support stored procedures.");
}

Dictionary CursorSQLite::parse_row(sqlite3_stmt *stmt)
{
    Dictionary row;

    // Get column count
    int col_count = sqlite3_column_count(stmt);

    // Fetch all columns
    for (int i = 0; i < col_count; i++)
    {
        // Key name
        const char *col_name = sqlite3_column_name(stmt, i);
        String key = String(col_name);

        // Value type
        int col_type = sqlite3_column_type(stmt, i);

        // Value
        Variant value;
        switch(col_type)
        {
            case SQLITE_INTEGER:
                value = Variant(sqlite3_column_int(stmt, i));
                break;
            
            case SQLITE_FLOAT:
                value = Variant(sqlite3_column_double(stmt, i));
                break;
            
            case SQLITE_TEXT: {
                int size = sqlite3_column_bytes(stmt, i);
                String str = String::utf8((const char *)sqlite3_column_text(stmt, i), size);
                value = Variant(str);
                break;
            }

            case SQLITE_BLOB: {
                PackedByteArray arr;
                int size = sqlite3_column_bytes(stmt, i);
                arr.resize(size);
                memcpy((void *)arr.ptr(), sqlite3_column_blob(stmt, i), size);
                value = Variant(arr);
                break;
            }

            default:
                break;
        }

        // Set dictionary value
        row[key] = value;
    }
    return row;
}

bool CursorSQLite::bind_parameters(sqlite3_stmt *stmt, Array arguments)
{
    int param_count = sqlite3_bind_parameter_count(stmt);
    int arg_count = arguments.size();
    if(param_count != arg_count)
    {
        print_error("SQLite statement expected " + itos(param_count) + " arguments, got " + itos(arg_count));
        return false;
    }
    for(int i = 0; i < param_count; i++)
    {
        int retcode;

        switch (arguments[i].get_type()) {
        case Variant::Type::NIL:
            retcode = sqlite3_bind_null(stmt, i + 1);
            break;
        case Variant::Type::BOOL:
        case Variant::Type::INT:
            retcode = sqlite3_bind_int(stmt, i + 1, (int)arguments[i]);
            break;
        case Variant::Type::FLOAT:
            retcode = sqlite3_bind_double(stmt, i + 1, (double)arguments[i]);
            break;
        case Variant::Type::STRING:
            retcode = sqlite3_bind_text(stmt, i + 1, String(arguments[i]).utf8().get_data(), -1, SQLITE_TRANSIENT);
            break;
        case Variant::Type::PACKED_BYTE_ARRAY:
            retcode = sqlite3_bind_blob(stmt, i + 1, PackedByteArray(arguments[i]).ptr(), PackedByteArray(arguments[i]).size(), SQLITE_TRANSIENT);
            break;
        default:
            print_error("SQLite was passed unhandled Variant with TYPE_* enum " + itos(arguments[i].get_type()) + ". Please serialize your object into a String or a PackedByteArray.\n");
            return false;
        }
        
        if (retcode != SQLITE_OK) {
			print_error("SQLite query failed, an error occured while binding argument" + itos(i + 1) + " of " + itos(arg_count) + " (" + sqlite3_errstr(retcode) + ")");
			return false;
		}
    }
    
    return true;
}

bool CursorSQLite::execute(String statement, Array arguments)
{
    ERR_FAIL_COND_V_MSG(!is_open(), false, "SQLite cursor is not open!");

    sqlite3_stmt *stmt = database->prepare_statement(statement.utf8().get_data());

    if(!stmt)
    {
        return false;
    }

    if(!bind_parameters(stmt, arguments))
    {
        sqlite3_finalize(stmt);
        return false;
    }

    last_result.clear();
    result_pos = 0;

    int err;
    do
    {
        err = sqlite3_step(stmt);

        if(err == SQLITE_ROW)
        {
            last_result.append(parse_row(stmt));
        }
    } while(err == SQLITE_ROW);

    if(err != SQLITE_DONE)
    {
        print_error(String("SQLite error: ") + sqlite3_errmsg(database->connection));
    }

    sqlite3_finalize(stmt);
    
    return err == SQLITE_DONE;
}

bool CursorSQLite::execute_many(String statement, Array arg_lists)
{
    ERR_FAIL_COND_V_MSG(!is_open(), false, "SQLite cursor is not open!");

    sqlite3_stmt *stmt = database->prepare_statement(statement.utf8().get_data());

    last_result.clear();
    result_pos = 0;

    for(int i = 0; i < arg_lists.size(); i++)
    {
        if(!bind_parameters(stmt, arg_lists[i]))
        {
            sqlite3_finalize(stmt);
            return false;
        }

        int err;
        do
        {
            err = sqlite3_step(stmt);
        } while(err == SQLITE_ROW);

        if(err != SQLITE_DONE)
        {
            print_error(String("SQLite error: ") + sqlite3_errmsg(database->connection));
            sqlite3_finalize(stmt);
            return false;
        }

        sqlite3_reset(stmt);
    }

    return true;
}

int CursorSQLite::get_row_count()
{
    return last_result.size();
}

void CursorSQLite::scroll(int amount, bool absolute)
{
    ERR_FAIL_COND_MSG(!is_open(), "SQLite cursor is not open!");

    if(absolute)
    {
        ERR_FAIL_COND(amount < 0);
        ERR_FAIL_COND(amount >= get_row_count());

        result_pos = amount;
    }
    else
    {
        ERR_FAIL_COND(result_pos + amount < 0);
        ERR_FAIL_COND(result_pos + amount >= get_row_count());

        result_pos += amount;
    }
}

Dictionary CursorSQLite::fetch_one()
{
    ERR_FAIL_COND_V_MSG(!is_open(), Dictionary(), "SQLite cursor is not open!");

    if(result_pos + 1 > get_row_count())
        return Dictionary();
    
    Dictionary row = last_result[result_pos];
    result_pos += 1;
    return row;
}

Array CursorSQLite::fetch_many(int size)
{
    ERR_FAIL_COND_V_MSG(!is_open(), Array(), "SQLite cursor is not open!");

    Array rows;
    if(result_pos >= last_result.size())
        return rows;

    rows = last_result.slice(result_pos, std::min(result_pos + size, last_result.size() - 1), 1, true);
    result_pos = std::min(result_pos + size, last_result.size() - 1);
    return rows;
}

Array CursorSQLite::fetch_all()
{
    ERR_FAIL_COND_V_MSG(!is_open(), Array(), "SQLite cursor is not open!");

    Array rows;
    if(result_pos >= get_row_count())
        return rows;
    
    rows = last_result.slice(result_pos, last_result.size() - 1, 1, true);
    result_pos = last_result.size() - 1;
    return rows;
}