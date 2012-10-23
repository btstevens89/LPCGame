#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <lua.hpp>
#include "src/debug.h"
#include "src/animatedimage.h"
#include "src/button.h"
#include "src/camera.h"
#include "src/color.h"
#include "src/entity.h"
#include "src/input.h"
#include "src/math.h"
#include "src/motionstate.h"
#include "src/physics.h"
#include "src/rect.h"
#include "src/state.h"
#include "src/statemanager.h"
#include "src/text.h"
#include "src/timer.h"
#include "src/vectors.h"
#include "src/window.h"
#include "src/luascript.h"
#include "luac/luacvector2f.h"
#include "luac/luacrectf.h"
#include "luac/luacphysics.h"
#include "luac/luacentity.h"
#include "luacscript.h"

const LuaC::LuaScriptLib::TLuaLibs LuaC::LuaScriptLib::sLuaLibs = LuaC::LuaScriptLib::CreateLibMap();
const LuaC::LuaScriptLib::TTableAdders LuaC::LuaScriptLib::sTableAdders = LuaC::LuaScriptLib::CreateAdderMap();

int LuaC::LuaScriptLib::stackDump(lua_State *l){
    std::stringstream ss;
    ss << "Stack: ";
    for (int i = 1, top = lua_gettop(l); i <= top; ++i){
        int t = lua_type(l, i);
        ss << "@" << i << ": ";
        switch (t) {
            //Strings
            case LUA_TSTRING:
                ss << lua_tostring(l, i);
                break;
            //Bools
            case LUA_TBOOLEAN:
                ss << (lua_toboolean(l, i) ? "True" : "False");
                break;
            //Numbers
            case LUA_TNUMBER:
                ss << lua_tonumber(l, i);
                break;
            //Other (userdata)
            default:
                ss << lua_typename(l, t) << "(" << readType(l, i) << ")";
                break;
        }
        ss << ", ";
    }
    ss << std::endl;
    Debug::Log(ss.str());
    return 0;
}
std::string LuaC::LuaScriptLib::readType(lua_State *l, int i){
    //Stack: some # of params, i corresponds to the udata we want to identify
    std::string type = "";
    //Get the metatable of udata at index i
    if (lua_getmetatable(l, i)){
        //Stack: stuff, udata's metatable
        //Get the "type" field
        lua_getfield(l, -1, "type");
        //Stack: stuff, udata metatable, typename
        //Get the type from the stack
        type = luaL_checkstring(l, -1);
    }
    //Stack: stuff, udata metatable, typename
    //Stack contains the typename and the metatable, pop them off
    lua_pop(l, 2);
    return type;
}
int LuaC::LuaScriptLib::requireLib(lua_State *l){
    //Try to look up the module desired, if it's one of ours load it, if not error
    std::string module = lua_tostring(l, -1);
    LuaC::LuaScriptLib::TLuaLibs::const_iterator fnd = sLuaLibs.find(module);
    if (fnd != sLuaLibs.end())
        lua_pushcfunction(l, sLuaLibs.at(module));
    else {
        std::string err = "requireLib Error: Failed to find: "
            + module + "\n";
        lua_pushstring(l, err.c_str());
    }
    return 1;
}
int LuaC::LuaScriptLib::requireScript(lua_State *l){
    //Check if the script is an engine script, if yes push DoScript, if no error
    std::string script = lua_tostring(l, -1);
    if (script.substr(0, 7) == "scripts"){
        std::string scriptFile = "../res/" + script;
        std::ifstream checkFile(scriptFile.c_str());
        if (checkFile.good()){
            checkFile.close();
            lua_pushstring(l, scriptFile.c_str());
            lua_pushcfunction(l, LuaC::LuaScriptLib::doScript);
        }
        else {
            std::string err = "LuaCScript::RequireScript Error: Failed to find: " 
                + scriptFile + "\n";
            std::cout << err << std::endl;
            lua_pushstring(l, err.c_str());
        }
    }
    //If not an engine script, nothing to return
    else 
        return 0;
    //If we found script or failed to load engine script we've got a result to return
    return 1;
}
std::vector<std::string> LuaC::LuaScriptLib::checkUserData(lua_State *l){
    //l stack: params
    //We want to step through and record the typenames of the userdata
    std::vector<std::string> udata;
    for (int i = 1, top = lua_gettop(l); i <= top; ++i){
        int t = lua_type(l, i);
        std::string luaTName = lua_typename(l, t);
        //If we find some userdata, read the type and store it
        if (luaTName == "userdata")
            udata.push_back(readType(l, i));
    }
    return udata;
}
void LuaC::LuaScriptLib::setUserData(lua_State *l, std::vector<std::string> types){
    //l stack: params
    //Step through and find udata, and register it according to the value at the vector
    //after each registration, increment vector pos
    std::vector<std::string>::const_iterator iter = types.begin();
    for (int i = 1, top = lua_gettop(l); i <= top; ++i && iter != types.end()){
        int t = lua_type(l, i);
        std::string luaTName = lua_typename(l, t);
        //If we find some userdata, read the type and register it accordingly
        if (luaTName == "userdata"){
            sTableAdders.at(*iter)(l, (i - top - 1));
            ++iter;
        }
    }
}
int LuaC::LuaScriptLib::LuaOpenLib(lua_State *l, const std::string &metatable,
    const std::string &className, const luaL_reg *lib, int (*call)(lua_State*))
{
    //Stack: lib name
    //Push the metatable to contain the fcns onto the stack
    luaL_newmetatable(l, metatable.c_str());
    //Copy metatable from -1 to the top
    lua_pushvalue(l, -1);
    //Set table at -2 key of __index = top of stack
    //ie. LPC.LuaRect.__index = table containing luaRectLib_m
    lua_setfield(l, -2, "__index");
    //Register the lib to the metatable at top of stack
    luaL_register(l, NULL, lib);
    //Stack: lib name, metatable
    //Add type identifier to the metatable
    lua_pushstring(l, className.c_str());
    lua_setfield(l, -2, "type");
    //Stack: lib name, metatable
    //Setup the LuaRect table, for making LuaRects
    lua_newtable(l);
    //Stack: lib name, metatable, table
    //Push the new fcn
    lua_pushcfunction(l, call);
    //Stack: lib name, metatable, table, newLuaRect fcn
    //Now newLuaRect fcn is @ key __call in the table
    lua_setfield(l, -2, "__call");
    //Stack: lib name, metatable, table
    //We want to set the table containing __call to be the metatable
    //of the LuaRect metatable
    lua_setmetatable(l, -2);
    //Stack: lib name, metatable
    //Name our metatable and make it global
    lua_setglobal(l, className.c_str());
    //Stack: lib name
    return 0;
}
void LuaC::LuaScriptLib::Add(lua_State *l, int i, const std::string &metatable){
    //Given stack containing unknown amount of things along with the udata
    //udata is at index i
    luaL_getmetatable(l, metatable.c_str());
    //Now stack is ??? with the metatable at top
    //So we know the index of our rect is bumped down 1 more so we adjust
    //and set the table
    lua_setmetatable(l, i - 1);
}
int LuaC::LuaScriptLib::doScript(lua_State *l){
    std::string script = lua_tostring(l, 0);
    luaL_dofile(l, script.c_str());
    return 0;
}
LuaC::LuaScriptLib::TLuaLibs LuaC::LuaScriptLib::CreateLibMap(){
    TLuaLibs map;
    map["AnimatedImage"] = &AnimatedImage::RegisterLua;
    map["Button"]        = &Button::RegisterLua;
    map["Camera"]        = &Camera::RegisterLua;
    map["Color"]         = &Color::RegisterLua;
    map["Debug"]         = &Debug::RegisterLua;
    map["Entity"]        = &Entity::RegisterLua;
    map["Image"]         = &Image::RegisterLua;
    map["Input"]         = &Input::RegisterLua;
    map["Math"]          = &Math::RegisterLua;
    map["MotionState"]   = &MotionState::RegisterLua;
    map["Physics"]       = &Physics::RegisterLua;
    map["Rect"]          = &Rectf::RegisterLua;
    map["State"]         = &State::RegisterLua;
    map["StateManager"]  = &StateManager::RegisterLua;
    map["Text"]          = &Text::RegisterLua;
    map["Timer"]         = &Timer::RegisterLua;
    map["Vector2"]       = &Vector2f::RegisterLua;
    map["Window"]        = &Window::RegisterLua;
    return map;
}
LuaC::LuaScriptLib::TTableAdders LuaC::LuaScriptLib::CreateAdderMap(){
    TTableAdders map;
    map["Entity"]   = &LuaC::EntityLib::addEntity;
    map["Rectf"]    = &LuaC::RectfLib::addRectf;
    //map["Physics"]  = &LuaC::PhysicsLib::addPhysics;
    map["Vector2f"] = &LuaC::Vector2fLib::addVector2f;
    return map;
}