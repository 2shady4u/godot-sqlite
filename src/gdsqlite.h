#ifndef SQLITE_CLASS_H
#define SQLITE_CLASS_H

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/viewport.hpp>

#include <godot_cpp/core/binder_common.hpp>

namespace godot
{
    class SQLite : public RefCounted
    {
        GDCLASS(SQLite, RefCounted)

    protected:
        static void _bind_methods();

    public:
        SQLite();
        ~SQLite();
    };

}

#endif // ! SQLITE_CLASS_H
