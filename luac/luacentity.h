#ifndef LUACENTITY_H
#define LUACENTITY_H

#include <string>
#include <memory>
#include <lua.hpp>
#include "src/entity.h"

///A namespace for storing the Lua C API code
/**
*  A namespace to store the various functions for interacting with
*  Lua via the Lua C API
*/
namespace LuaC {
    ///The Entity metatable name
    const std::string entityMeta = "LPC.Entity";
    ///The Entity class/type name
    const std::string entityClass = "Entity";
    /**
    *  Class for storing functions used to manage interaction
    *  between Lua and the Entity class. Defines the Entity Lua Lib
    */
    class EntityLib {
    public:
        ///Open the Entity Lua library for Lua state l
        static int luaopen_entity(lua_State *l);
        /**
        *  Add the Entity metatable to the userdata at index i
        *  i is relative to top (ie. -1 is top)
        *  @param l The Lua state
        *  @param i The index of the userdata to add (index relative to top, neg #'s)
        */
        static void addEntity(lua_State *l, int i);
        /**
        *  Check if the userdata at index i in the stack is an Entity
        *  and return a pointer to it
        *  @param l The Lua state
        *  @param i The index of the userdata (standard index, pos #'s)
        */
        //static Entity** checkEntity(lua_State *l, int i);
        static std::shared_ptr<Entity>* checkEntity(lua_State *l, int i);

    private:
        ///The Lua function library struct
        static const struct luaL_reg luaEntityLib[];
        ///Make a new Entity in Lua state l
        static int newEntity(lua_State *l);
        ///Call a function on the Entity's script
        static int callFunction(lua_State *caller);
        //Destroy the Entity, removing it from the manager and existence
        static int destroy(lua_State *l);
        ///Getters
        static int getPhysics(lua_State *l);
        static int getBox(lua_State *l);
        static int getTag(lua_State *l);
        static int getName(lua_State *l);
        ///__newindex accessor for setters
        static int newIndex(lua_State *l);
        ///Setters
        ///Set the entity tag, i corresponds to the index to get the value from
        static int setTag(lua_State *l, int i);
        ///Operators
        static int toString(lua_State *l);
        static int concat(lua_State *l);
        ///Garbage collection
        static int garbageCollection(lua_State *l);
    };
};

#endif