#ifndef GODOT_DATABASE_H
#define GODOT_DATABASE_H

#include "core/reference.h"
#include "cursor.h"

class Database : public Reference {
    GDCLASS(Database, Reference);

    protected:
    static void _bind_methods();

    public:
    virtual bool is_open() = 0;

    /// Close the database connection, rendering it invalid for further use.
    ///
    /// For databases that implement transactions, closing
    /// the connection without committing the last transaction
    /// should trigger an implicit rollback.
    virtual void close() = 0;

    /// Commit any pending transaction to the database.
    /// For databases that implement auto-commit, it should
    /// be disabled by default, with an interface method
    /// to re-enable it.
    ///
    /// For databases that don't implement transactions, this
    /// method can print a warning and do nothing.
    virtual void commit() = 0;

    /// Rollback the effects of the current transaction.
    ///
    /// For databases that don't implement transactions, this
    /// method should print an error and do nothing.
    virtual void rollback() = 0;

    /// Return a cursor to the database.
    virtual Ref<Cursor> cursor() = 0;
};

#endif