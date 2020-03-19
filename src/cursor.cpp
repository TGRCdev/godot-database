#include "cursor.h"

void Cursor::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("is_open"), &Cursor::is_open);
    ClassDB::bind_method(D_METHOD("close"), &Cursor::close);
    ClassDB::bind_method(D_METHOD("callproc", "procname", "arguments"), &Cursor::callproc, DEFVAL(Array()));
    ClassDB::bind_method(D_METHOD("execute", "statement", "arguments"), &Cursor::execute, DEFVAL(Array()));
    ClassDB::bind_method(D_METHOD("execute_many", "statement", "arguments"), &Cursor::execute_many);
    ClassDB::bind_method(D_METHOD("get_row_count"), &Cursor::get_row_count);
    ClassDB::bind_method(D_METHOD("scroll", "amount", "absolute"), &Cursor::scroll, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("fetch_one"), &Cursor::fetch_one);
    ClassDB::bind_method(D_METHOD("fetch_many", "size"), &Cursor::fetch_many);
    ClassDB::bind_method(D_METHOD("fetch_all"), &Cursor::fetch_all);
}