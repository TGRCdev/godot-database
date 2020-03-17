#ifndef GODOT_DATABASE_H
#define GODOT_DATABASE_H

#include "core/reference.h"
#include "core/ustring.h"

class Database : public Reference {
    GDCLASS(Database, Reference);

    protected:
    static void _bind_methods();

    bool _opened;

    public:
    bool is_open() const {return _opened;}
    virtual bool query(String statement) = 0;
    virtual void close() = 0;
};

#endif