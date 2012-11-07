#include <memory>
#include <lua.hpp>
#include "src/entity.h"
#include "src/physics.h"
#include "src/rect.h"
#include "src/entitymanager.h"
#include "src/statemanager.h"
#include "src/state.h"
#include "src/entitymanager.h"
#include "src/debug.h"
#include "luacscript.h"
#include "luacrectf.h"
#include "luacvector2f.h"
#include "luacphysics.h"
#include "luacentity.h"

int LuaC::EntityLib::luaopen_entity(lua_State *l){
    return LuaScriptLib::LuaOpenLib(l, entityMeta, entityClass, luaEntityLib, newEntity);
}
void LuaC::EntityLib::addEntity(lua_State *l, int i){
    LuaScriptLib::Add(l, i, entityMeta);
}
/*
Entity** LuaC::EntityLib::checkEntity(lua_State *l, int i){
    return (Entity**)luaL_checkudata(l, i, entityMeta.c_str());
}
*/
std::shared_ptr<Entity>* LuaC::EntityLib::checkEntity(lua_State *l, int i ){
    return (std::shared_ptr<Entity>*)luaL_checkudata(l, i, entityMeta.c_str());
}
const struct luaL_reg LuaC::EntityLib::luaEntityLib[] = {
    { "callFunction", callFunction },
    { "destroy", destroy },
    { "physics", getPhysics },
    { "box", getBox },
    { "tag", getTag },
    { "name", getName },
    { "__newindex", newIndex },
    { "__tostring", toString },
    { "__concat", concat },
    { "__gc", garbageCollection },
    { NULL, NULL }
};
int LuaC::EntityLib::newEntity(lua_State *l){
    //Stack: class table, entity file
    std::string file = luaL_checkstring(l, 2);
    //Make a new Entity and register it with the manager
    //Entity *e = new Entity(file);
    std::shared_ptr<Entity> e(new Entity(file));
    e->Init();
    //Register the Entity with the State
    std::shared_ptr<EntityManager> manager = StateManager::GetActiveState()->Manager();
    manager->Register(e);
    //Make the userdata
    std::shared_ptr<Entity> *luaE = (std::shared_ptr<Entity>*)lua_newuserdata(l, sizeof(std::shared_ptr<Entity>));
    //Entity **luaE = (Entity**)lua_newuserdata(l, sizeof(Entity*));
    //*luaE = e;
    *luaE = e;
    addEntity(l, -1);
    return 1;
}
int LuaC::EntityLib::callFunction(lua_State *caller){
    /*
    *  Caller stack:
    *  udata             - The Entity udata we want to call the function on
    *  string (fcn name) - The function we want to call
    *  int (# results)   - # of results to return
    *  params            - All remaining values on the stack are the params to pass
    */
    //Get the lua_State of the Entity we want to call the function on
    //Entity **e = checkEntity(caller, 1);
    std::shared_ptr<Entity>* e = checkEntity(caller, 1);
    lua_State *reciever = (*e)->Script()->Get();
    //Get function name and # results
    std::string fcnName = luaL_checkstring(caller, 2);
    int nRes = luaL_checkint(caller, 3);
    ///Remove the udata, fnc name and # results values from the stack
    for (int i = 0; i < 3; ++i)
        lua_remove(caller, 1);
    //Caller stack: params
    //# params = caller stack size
    int nParam = lua_gettop(caller);

    //Lookup userdata types of the params
    std::vector<std::string> udataTypes = LuaScriptLib::checkUserData(caller);
    //Get the function in reciever
    lua_getglobal(reciever, fcnName.c_str());
    //Reciever stack: function
    //Transfer params
    //TODO: Copy, don't move! Or it just the metatables that are being lost in the sender?
    lua_xmove(caller, reciever, nParam);
    //Caller stack: Empty
    //Reciever stack: params
    //Restore userdata metatables
    LuaScriptLib::setUserData(reciever, udataTypes);
    //Call the function
    if (lua_pcall(reciever, nParam, nRes, 0) != 0){
        Debug::Log("Error calling: " + fcnName + " " + lua_tostring(reciever, -1));
        return 0;
    }
    //Reciever stack: results
    //Read result userdata types
    udataTypes = LuaScriptLib::checkUserData(reciever);
    //Transfer results
    //TODO: Copy, don't move! Or it just the metatables that are being lost in the sender?
    lua_xmove(reciever, caller, nRes);
    //Restore userdata metatables
    LuaScriptLib::setUserData(caller, udataTypes);
    /*
    *  Final stacks:
    *  Caller: results
    *  Reciever: empty
    */
    return nRes;
}
int LuaC::EntityLib::destroy(lua_State *l){
    //Stack: udata (Entity) to be removed
    std::shared_ptr<Entity>* e = checkEntity(l, 1);
    if (*e != nullptr){
        std::cout << "Will try to destroy entity: " << (*e)->Name() << std::endl;
        //Remove it from the manager
        std::shared_ptr<EntityManager> manager = StateManager::GetActiveState()->Manager();
        manager->Remove(*e);
        //Come up with better way to find entity in the manager?
        e->reset();
    }
    else
        std::cout << "Entity alread destroyed" << std::endl;
    return 0;
}
int LuaC::EntityLib::getPhysics(lua_State *l){
    //Stack: udata (Entity)
    //Entity **e = checkEntity(l, 1);
    std::shared_ptr<Entity>* e = checkEntity(l, 1);
    //Make a new Physics userdata
    Physics** luaP = (Physics**)lua_newuserdata(l, sizeof(Physics*));
    //Give it the Physics metatable
    PhysicsLib::addPhysics(l, -1);
    //Set it to the entity's physics
    *luaP = (*e)->GetPhysics();
    return 1;
}
int LuaC::EntityLib::getBox(lua_State *l){
    //Stack: udata (Entity)
    //Entity **e = checkEntity(l, 1);
    std::shared_ptr<Entity>* e = checkEntity(l, 1);
    //Make a new Rectf
    Rectf *r = (Rectf*)lua_newuserdata(l, sizeof(Rectf));
    //Give it the Rectf metatable
    RectfLib::addRectf(l, -1);
    //Is this correct?
    *r = (*e)->Box();
    return 1;
}
int LuaC::EntityLib::getTag(lua_State *l){
    //Stack: udata (Entity)
    //Entity **e = checkEntity(l, 1);
    std::shared_ptr<Entity>* e = checkEntity(l, 1);
    lua_pushstring(l, (*e)->Tag().c_str());
    return 1;
}
int LuaC::EntityLib::getName(lua_State *l){
    //Stack: udata (Entity)
    //Entity **e = checkEntity(l, 1);
    std::shared_ptr<Entity>* e = checkEntity(l, 1);
    lua_pushstring(l, (*e)->Name().c_str());
    return 1;
}
int LuaC::EntityLib::newIndex(lua_State *l){
    //Stack: udata (Entity), string of index to set, val to set as
    std::string index = luaL_checkstring(l, 2);
    switch(index.at(0)){
        case 't':
            return setTag(l, 3);
        default:
            break;
    }
    return 0;
}
int LuaC::EntityLib::setTag(lua_State *l, int i){
    //Stack: udata, ??? with tag @ i
    //Entity **e = checkEntity(l, 1);
    std::shared_ptr<Entity>* e = checkEntity(l, 1);
    std::string tag = luaL_checkstring(l, i);
    (*e)->SetTag(tag);
    return 0;
}
int LuaC::EntityLib::toString(lua_State *l){
    return 1;
}
int LuaC::EntityLib::concat(lua_State *l){
    return 1;
}
int LuaC::EntityLib::garbageCollection(lua_State *l){
    //Unsure of how to make this be called when it goes out of scope, if we exit
    //a state and these entity shared_ptrs aren't cleaned up we'll leak some entitys i think..
    //Especially if a script holds its own entity shared_ptr, it wouldn't delete itself?
    //Stack: udata
    std::cout << "Resetting entity shared_ptr" << std::endl;
    //Should it also be removed from the manager? hmm
    std::shared_ptr<Entity>* e = checkEntity(l, 1);
    e->reset();
    e = nullptr;
    return 0;
}
