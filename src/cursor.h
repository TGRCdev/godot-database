#ifndef GODOT_CURSOR_H
#define GODOT_CURSOR_H

#include "core/reference.h"
#include "core/ustring.h"

class Cursor : public Reference {
    GDCLASS(Cursor, Reference);

    protected:
    static void _bind_methods();

    public:
    // Returns true if the cursor is still valid and connected to a database
    virtual bool is_open() = 0;
    
    // Close the database cursor, rendering it invalid for further use.
    virtual void close() = 0;

    /// Call a stored procedure with the given name.
    /// OPTIONAL. Should print an error if it is not supported.
    virtual bool callproc(String procname, Array arguments) = 0;

    /// Prepare and execute a database statement with the given arguments.
    virtual bool execute(String statement, Array arguments) = 0;

    /// Prepares a database statement and executes it with the given Array of Arrays of arguments.
    virtual bool execute_many(String statement, Array arguments) = 0;

    /// Returns the number of rows affected by the last query.
    /// OPTIONAL. Returns -1 if the interface cannot determine the number of rows affected.
    virtual int get_row_count() {return -1;}

    /// Moves forward or backward in the result set.
    /// OPTIONAL. Should print an error if not supported.
    virtual void scroll(int amount, bool absolute) = 0;
    
    /// Returns a single row from the last result as a Dictionary.
    /// Returns an empty Dictionary if there were no results, or the cursor reached the end
    /// of the results set.
    virtual Dictionary fetch_one() = 0;

    /// Returns an Array of Dictionary rows.
    /// Returns an empty Array if there were no results, or the cursor reached the end
    /// of the result set.
    virtual Array fetch_many(int size) = 0;

    /// Returns the remaining result rows from the last query as an Array of Dictionaries.
    /// Returns an empty Array if there were no results.
    virtual Array fetch_all() = 0;
};

#endif