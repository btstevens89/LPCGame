#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <lua.hpp>
#include "core/debug.h"
#include "core/luascript.h"
#include "luacvector2f.h"
#include "luacrectf.h"
#include "luacphysics.h"
#include "luacentity.h"
#include "luacdebug.h"
#include "luaccolor.h"
#include "luactimer.h"
#include "luacinput.h"
#include "luacstate.h"
#include "luaccamera.h"
#include "luacmath.h"
#include "luacimage.h"
#include "luacwindow.h"
#include "luacanimatedimage.h"
#include "luactext.h"
#include "luacscript.h"

const LuaC::LuaScriptLib::TLuaLibs LuaC::LuaScriptLib::sLuaLibs = LuaC::LuaScriptLib::CreateLibMap();
const LuaC::LuaScriptLib::TUdataCopiers LuaC::LuaScriptLib::sUdataCopiers = LuaC::LuaScriptLib::CreateCopierMap();

int LuaC::LuaScriptLib::StackDump(lua_State *l, bool toLog){
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
            //Userdata
            case LUA_TUSERDATA:
                ss << lua_typename(l, t) << " (" << readType(l, i) << ")";
                break;
            //Other (tables/etc)
            default:
                ss << lua_typename(l, t);
                break;
        }
        ss << ", ";
    }
    if (toLog)
        Debug::Log(ss.str());
    else 
        std::cout << ss.str() << std::endl;

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
        if (lua_isstring(l, -1))
            type = luaL_checkstring(l, -1);
        //Stack: stuff, udata metatable, typename
        //Stack contains the typename and the metatable
        //pop them off to restore original stack state
        lua_pop(l, 2);
    }
    return type;
}
int LuaC::LuaScriptLib::requireLib(lua_State *l){
    //Try to look up the module desired, if it's one of ours load it, if not error
    std::string module = lua_tostring(l, -1);
    TLuaLibs::const_iterator fnd = sLuaLibs.find(module);
    if (fnd != sLuaLibs.end())
        lua_pushcfunction(l, fnd->second);
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
void LuaC::LuaScriptLib::CopyStack(lua_State *sender, lua_State *reciever, int numVals){
    /*
    *  Description of how we properly loop through the stack to preserve
    *  the ordering of the data
    *  stack: 10, "string", udata, 45
    *  numVals = 2
    *  top = 4
    *  45 @ -1 ie. @ 4
    *  want to pass 4 & 3 starting at 3
    *  so loop from top - (numVals - 1) to top
    */
    int top = lua_gettop(sender);
    for (int i = top - (numVals - 1); i < top + 1; ++i)
        CopyData(sender, i, reciever);
}
void LuaC::LuaScriptLib::CopyData(lua_State *sender, int idx, lua_State *reciever){
    switch (lua_type(sender, idx)) {
        //Strings
        case LUA_TSTRING:
            lua_pushstring(reciever, lua_tostring(sender, idx));
            break;
        //Bools
        case LUA_TBOOLEAN:
            lua_pushboolean(reciever, lua_toboolean(sender, idx));
            break;
        //Numbers
        case LUA_TNUMBER:
            lua_pushnumber(reciever, lua_tonumber(sender, idx));
            break;
        //Userdata
        case LUA_TUSERDATA:
            CopyUdata(sender, idx, reciever);
            break;
        //Other (tables/etc)
        //How to copy tables?
        default:
            break;
    }
}
void LuaC::LuaScriptLib::CopyUdata(lua_State *sender, int idx, lua_State *reciever){
    std::string type = readType(sender, idx);
    TUdataCopiers::const_iterator fnd = sUdataCopiers.find(type);
    if (fnd != sUdataCopiers.end())
        fnd->second(sender, idx, reciever);
    else 
        Debug::Log("LuaScriptLib::CopyUdata: Failed to find userdata copier for type: " + type);
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
    map[animatedImageClass] = &AnimatedImageLib::luaopen_animatedimage;
    map[cameraClass]    = &CameraLib::luaopen_camera;
    map[colorClass]     = &ColorLib::luaopen_color;
    map[debugClass]     = &DebugLib::luaopen_debug;
    map[entityClass]    = &EntityLib::luaopen_entity;
    map[imageClass]     = &ImageLib::luaopen_image;
    map[inputClass]     = &InputLib::luaopen_input;
    map[luaScriptClass] = &LuaScriptLib::luaopen_luascript;
    map[mathClass]      = &MathLib::luaopen_math;
    map[physicsClass]   = &PhysicsLib::luaopen_physics;
    map[rectfClass]     = &RectfLib::luaopen_rectf;
    map[stateClass]     = &StateLib::luaopen_state;
    map[textClass]      = &TextLib::luaopen_text;
    map[timerClass]     = &TimerLib::luaopen_timer;
    map[vector2fClass]  = &Vector2fLib::luaopen_vector2f;
    map[windowClass]    = &WindowLib::luaopen_window;

    return map;
}
LuaC::LuaScriptLib::TUdataCopiers LuaC::LuaScriptLib::CreateCopierMap(){
    TUdataCopiers map;
    map[vector2fClass] = &Vector2fLib::Copy;
    map[colorClass]    = &ColorLib::Copy;
    map[rectfClass]    = &RectfLib::Copy;
    map[timerClass]    = &TimerLib::Copy;
    map[physicsClass]  = &PhysicsLib::Copy;
    map[entityClass]   = &EntityLib::Copy;
    map[cameraClass]   = &CameraLib::Copy;
    map[imageClass]    = &ImageLib::Copy;
    map[animatedImageClass] = &AnimatedImageLib::Copy;
    map[textClass]     = &TextLib::Copy;
    return map;
}
const luaL_reg LuaC::LuaScriptLib::luaScriptLib[] = {
    { "stackDump", stackDump },
    { NULL, NULL}
};
int LuaC::LuaScriptLib::luaopen_luascript(lua_State *l){
    luaL_register(l, luaScriptClass.c_str(), luaScriptLib);
    return 0;
}
int LuaC::LuaScriptLib::stackDump(lua_State *l){
    //Stack: the lua script table, bool of whether to 
    //dump to file, and the data to check
    bool toLog = true;
    //If a bool flag for toLog is passed, get it and remove it
    //so it won't show up in the stack dump (this is a rare case where remove is ok!)
    if (lua_isboolean(l, 1)){
        toLog = (lua_toboolean(l, 1) == 1);
        lua_remove(l, 1);
    }
    StackDump(l, toLog);
    return 0;
}
