#ifndef LUACUDATA_H
#define LUACUDATA_H

#include <string>
#include <lua.hpp>
#include <memory>
#include "luaref.h"
#include "luacscript.h"

///A namespace for storing the Lua C API code
/**
*  A namespace to store the various functions for interacting with
*  Lua via the Lua C API
*/
namespace LuaC {
    /**
    * A base template class for inherting methods for performing operations
    * on objects in Lua such as getting, pushing and copying between states
    */
    template<class T>
    class DataLib {
    public:
        /**
        * Allocate memory for an instance of T in the Lua state, initialize it, 
        * assign the appropriate metatable and return a pointer to the object
        * @param l The Lua state to make the object in
        * @return pointer to the allocated object
        */
        static T* Allocate(lua_State *l){
            void *block = lua_newuserdata(l, sizeof(T));
            T *obj = new(block) T();
            AddMetaTable(l, -1);
            return obj;
        }
        /**
        * Push a copy of an object of type T onto the stack of the Lua state
        * @param obj The object to push
        * @param l The Lua state to push onto
        */
        static void Push(lua_State *l, const T obj){
            T *o = Allocate(l);
            *o = obj;
        }
        /**
        * Push a copy of an object of type T onto the stack as a global with some name
        * if the name is of the form table:field it'll be pushed onto the table under that field
        * @param obj The object to push
        * @param l The Lua state to push onto
        * @param name The global name to assign the object
        */
        static void Push(lua_State *l, const T obj, std::string name){
            if (TableField(name))
                PushField(l, obj, name);
            else {
                Push(l, obj);
                lua_setglobal(l, name.c_str());
            }
        }
        /**
        * Copy an object of type T from one Lua state at index i to another Lua state
        * @param from The Lua state to copy from
        * @param idx The index of the data in the from stack
        * @param too The Lua state to copy too
        */
        static void Copy(lua_State *from, int idx, lua_State *too){
            T* obj = Check(from, idx);
            Push(too, *obj);
        }
        /**
        * Check if the userdata at some index is of type T and return it
        * @param l The Lua state
        * @param i The index of the userdata
        */
        static T* Check(lua_State *l, int i){
            return (T*)luaL_checkudata(l, i, mMetaTable.c_str());
        }
        /**
        * Check if the userdata at some index is of type T and return a copy of it
        * @param l The Lua state
        * @param i Index of the data
        */
        static T GetCopy(lua_State *l, int i){
            return *(T*)luaL_checkudata(l, i, mMetaTable.c_str());
        }

    private:
        /**
        *  Add the metatable for the type to the userdata at index i relative to top
        *  @param l The Lua state the data is in
        *  @param i The index of the userdata to register, idx relative to top (negative #'s)
        */
        static void AddMetaTable(lua_State *l, int i){
            LuaScriptLib::Add(l, i, mMetaTable);
        }
        /**
        * Check if we're pushing onto a table ie, if there's a : in the name such as table:field
        * @param str The string to check if it's a table:field string
        * @return True if we're pushing a table value
        */
        static bool TableField(std::string str){
            return (str.find_first_of(':') != -1);
        }
        /**
        * Push a value on as a table field entry, using the string table:field and push
        * the value onto the table under the desired field
        * @param l The Lua state
        * @param obj The object to push
        * @param str The table:field formatted string
        */
        static void PushField(lua_State *l, T obj, std::string str){
            size_t colon = str.find_first_of(':');
            std::string table = str.substr(0, colon);
            std::string field = str.substr(colon + 1, str.size() - colon + 1);
            lua_getglobal(l, table.c_str());
            Push(l, obj);
            lua_setfield(l, -2, field.c_str());
            lua_pop(l, 1);
        }

    protected:
        ///The metatable name, value is set via template specialization for the desired class
        static const std::string mMetaTable;
    };
    template<class T>
    const std::string DataLib<T>::mMetaTable = "";

    //Specialization for Lua References
    typedef DataLib<LuaRef> LuaRefLib;
    LuaRef* LuaRefLib::Allocate(lua_State *l){
        return nullptr;
    }
    void LuaRefLib::Push(lua_State *l, const LuaRef obj){
        obj.Push(l);
    }
    void LuaRefLib::Push(lua_State *l, const LuaRef obj, std::string name){
        obj.Push(l, name);
    }
    void LuaRefLib::Copy(lua_State *from, int idx, lua_State *too){
    }
    LuaRef* LuaRefLib::Check(lua_State *l, int i){
        return nullptr;
    }
    LuaRef LuaRefLib::GetCopy(lua_State *l, int i){
        return LuaC::LuaRef();
    }
   
    //Specialization for primitive types
    typedef DataLib<int> IntLib;
    template<>
    static void IntLib::Push(lua_State *l, const int obj){
        lua_pushinteger(l, obj);
    }
    template<>
    static void IntLib::Copy(lua_State *from, int idx, lua_State *too){
        int val = luaL_checkint(from, idx);
        Push(too, val);
    }
    template<>
    static int IntLib::GetCopy(lua_State *l, int i){
        return luaL_checkint(l, i);
    }

    typedef DataLib<float> FloatLib;
    template<>
    static void FloatLib::Push(lua_State *l, const float obj){
        lua_pushnumber(l, obj);
    }
    template<>
    static void FloatLib::Copy(lua_State *from, int idx, lua_State *too){
        float val = luaL_checknumber(from, idx);
        Push(too, val);
    }
    template<>
    static float FloatLib::GetCopy(lua_State *l, int i){
        return luaL_checknumber(l, i);
    }

    typedef DataLib<double> DoubleLib;
    template<>
    static void DoubleLib::Push(lua_State *l, const double obj){
        lua_pushnumber(l, obj);
    }
    template<>
    static void DoubleLib::Copy(lua_State *from, int idx, lua_State *too){
        double val = luaL_checknumber(from, idx);
        Push(too, val);
    }
    template<>
    static double DoubleLib::GetCopy(lua_State *l, int i){
        return luaL_checknumber(l, i);
    }

    typedef DataLib<bool> BoolLib;
    template<>
    static void BoolLib::Push(lua_State *l, const bool obj){
        lua_pushboolean(l, obj);
    }
    template<>
    static void BoolLib::Copy(lua_State *from, int idx, lua_State *too){
        bool val = luaL_checknumber(from, idx);
        Push(too, val);
    }
    template<>
    static bool BoolLib::GetCopy(lua_State *l, int i){
        if (!lua_isboolean(l, i))
            return false;
        return (lua_toboolean(l, i) == 1);
    }

    typedef DataLib<std::string> StringLib;
    template<>
    static void StringLib::Push(lua_State *l, const std::string obj){
        lua_pushstring(l, obj.c_str());
    }
    template<>
    static void StringLib::Copy(lua_State *from, int idx, lua_State *too){
        std::string str = luaL_checkstring(from, idx);
        Push(too, str);        
    }
    template<>
    static std::string StringLib::GetCopy(lua_State *l, int i){
        return luaL_checkstring(l, i);
    }
    //String literals like "hello" are char *
    typedef DataLib<const char*> CharLib;
    template<>
    static void CharLib::Push(lua_State *l, const char *obj){
        lua_pushstring(l, obj);
    }
    template<>
    static void CharLib::Copy(lua_State *from, int idx, lua_State *too){
        std::string str = luaL_checkstring(from, idx);
        Push(too, str.c_str());        
    }
    template<>
    static const char* CharLib::GetCopy(lua_State *l, int i){
        return luaL_checkstring(l, i);
    }
}

#endif