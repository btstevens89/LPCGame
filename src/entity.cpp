#include <string>
#include <lua.hpp>
#include <luabind/luabind.hpp>
#include <fstream>
#include "luac/luaccamera.h"
#include "math.h"
#include "luascript.h"
#include "jsonhandler.h"
#include "debug.h"
#include "luac/luacudataparam.h"
#include "luac/luacprimitiveparam.h"
#include "entity.h"

Entity::Entity() : mPhysics(new Physics()), mName(""), mTag(""), mConfigFile(""),  mMouseOver(false), mClicked(false), mRender(true), mUiElement(false) 
{
}
Entity::Entity(std::string file) : mPhysics(new Physics()), mName(""), mTag(""), mConfigFile(""), mMouseOver(false), mClicked(false), mRender(true), mUiElement(false)
{
    Load(file);
}
Entity::~Entity(){
    std::cout << "Entity: " << mName << " destructor" << std::endl;
    //Make sure Free is called 
    Free();
}
void Entity::Init(std::shared_ptr<Entity> self){
	//We catch exceptions so that if the function doesn't exist the program 
	//won't crash. This lets us skip implementing functions we don't need
	//int scripts
	if (!mScript.Open())
		return;
    
    if (self != nullptr){
        //Push the self pointer onto the state so the script can use it
        LuaC::EntityParam luaSelf(&self);
        luaSelf.Push(mScript.Get(), "self");
    }
    /*
    std::shared_ptr<Entity> self(this);
    std::weak_ptr<Entity> selfWeak = self;
    LuaC::EntityParam globalSelf(&selfWeak);
    globalSelf.Push(mScript.Get(), "self");
    */
    mScript.CallFunction("Init");
}
void Entity::Free(){
    std::cout << "Entity: " << mName << " free" << std::endl;
	if (!mScript.Open()){
        std::cout << "mscript closed" << std::endl;
		return;
    }
    std::cout << "mscript open" << std::endl;
    mScript.CallFunction("Free");
    mScript.Close();
}
void Entity::Update(){
    //Should deltaT be passed to update instead?
	if (!mScript.Open())
		return;
	try {
		luabind::call_function<void>(mScript.Get(), "Update");
	}
	catch(...){
	}
    //Should we call Physics::Move here?
}
void Entity::Move(float deltaT){
	if (!mScript.Open())
		return;
	try {
		luabind::call_function<void>(mScript.Get(), "Move", deltaT);
	}
	catch(...){
	}
}
void Entity::Draw(std::weak_ptr<Camera> camera){
    //Draw entity
	if (!mScript.Open())
		return;
	try {
        lua_State *l = mScript.Get();
        lua_getglobal(l, "Draw");
        LuaC::CameraLib::Push(l, &camera);
        if (lua_pcall(l, 1, 0, 0) != 0)
            Debug::Log("Error calling Draw: ");// + lua_tostring(l, -1));
        
        //luabind::call_function<void>(mScript.Get(), "Draw", camera);
	}
	catch(...){
	}
}
void Entity::OnMouseDown(){
    mClicked = true;
	if (!mScript.Open())
		return;
	try {
		luabind::call_function<void>(mScript.Get(), "OnMouseDown");
	}
	catch(...){
	}
}
void Entity::OnMouseUp(){
    //We have to do this here for now b/c ObjectButtons don't have 
    //an attached script, however I'd prefer to call OnClick after OnMouseUp
    if (mClicked)
        OnClick();
    mClicked = false;
	
    if (!mScript.Open())
		return;
	try {
		luabind::call_function<void>(mScript.Get(), "OnMouseUp");
	}
	catch(...){
	}
}
void Entity::OnClick(){
    if (!mScript.Open())
        return;
    try {
        luabind::call_function<void>(mScript.Get(), "OnClick");
    }
    catch(...){
    }
}
void Entity::OnMouseEnter(){
    mMouseOver = true;
	if (!mScript.Open())
		return;
	try{
		luabind::call_function<void>(mScript.Get(), "OnMouseEnter");
	}
	catch(...){
	}
}
void Entity::OnMouseExit(){
    mClicked = false;
    mMouseOver = false;
	if (!mScript.Open())
		return;
	try {
		luabind::call_function<void>(mScript.Get(), "OnMouseExit");
	}
	catch(...){
	}
}
void Entity::CheckMouseOver(const Vector2f &pos){
	//Only trigger OnMouseEnter if the mouse is colliding and wasn't before
	if (Math::CheckCollision(pos, mPhysics->Box()) && !mMouseOver){
		OnMouseEnter();
		mMouseOver = true;
	}
	//Only trigger mouse exit if the mouse was colliding, but isn't anymore
	else if (!Math::CheckCollision(pos, mPhysics->Box()) && mMouseOver){
		OnMouseExit();
		mMouseOver = false;
	}
}
bool Entity::GetMouseOver() const {
	return mMouseOver;
}
Physics* Entity::GetPhysics(){
	return mPhysics.get();
}
std::weak_ptr<Physics> Entity::GetPhysicsWeakPtr(){
    std::weak_ptr<Physics> weak = mPhysics;
    return weak;
}
void Entity::SetCollisionMap(CollisionMap map){
	mPhysics->SetMap(map);
}
Rectf Entity::Box() const {
	return mPhysics->Box();
}
void Entity::SetTag(std::string tag){
	mTag = tag;
}
std::string Entity::Tag() const {
	return mTag;
}
void Entity::SetName(std::string name){
    mName = name;
}
std::string Entity::Name() const {
    return mName;
}
void Entity::Render(bool b){
    mRender = b;
}
bool Entity::Render() const {
    return mRender;
}
void Entity::IsUiElement(bool b){
    mUiElement = b;
}
bool Entity::IsUiElement() const {
    return mUiElement;
}
LuaScript* Entity::Script(){
    return &mScript;
}
Json::Value Entity::Save() const {
	//How to specify overrides to save?
    Json::Value val;
    if (mConfigFile != "")
        val["file"] = mConfigFile;
    else {
	    val["image"]   = mImage.File();
	    val["physics"] = mPhysics->Save();
	    val["tag"]	   = mTag;
	    val["script"]  = mScript.File();
	    val["name"]    = mName;
        val["render"]  = mRender;
        val["ui"]      = mUiElement;
    }
	return val;
}
void Entity::Save(const std::string &file) const {
    //Eventually will be merged in some manner with the other 
    //save function
    Json::Value val;
    val["image"]   = mImage.File();
	val["physics"] = mPhysics->Save();
	val["tag"]	   = mTag;
	val["script"]  = mScript.File();
	val["name"]    = mName;
    val["render"]  = mRender;
    val["ui"]      = mUiElement;
    JsonHandler handler(file);
    handler.Write(val);
}
void Entity::Load(Json::Value val){
    //Process overrides as well
	mTag  = val["tag"].asString();
	mName = val["name"].asString();
	mPhysics->Load(val["physics"]);
    mImage.Load(val["image"].asString());
	mScript.OpenScript(val["script"].asString());
    mRender = val["render"].asBool();
    mUiElement = val["ui"].asBool();
}
void Entity::Load(const std::string &file, Json::Value overrides){
    mConfigFile = file;
    JsonHandler handler(mConfigFile);
    Json::Value data = handler.Read();
    data["overrides"] = overrides;
    Load(data);
}
int Entity::RegisterLua(lua_State *l){
	using namespace luabind;

	module(l, "LPC")[
		class_<Entity>("Entity")
			.def(constructor<>())
			.def(constructor<std::string>())
			.def("Init", &Entity::Init)
			.def("Free", &Entity::Free)
			.def("Update", &Entity::Update)
			.def("Move", &Entity::Move)
			.def("Draw", &Entity::Draw)
			.def("OnMouseDown", &Entity::OnMouseDown)
			.def("OnMouseUp", &Entity::OnMouseUp)
			.def("OnMouseEnter", &Entity::OnMouseEnter)
			.def("OnMouseExit", &Entity::OnMouseExit)
			.def("GetPhysics", &Entity::GetPhysics)
			.def("Box", &Entity::Box)
			.def("SetTag", &Entity::SetTag)
			.def("Tag", &Entity::Tag)
            .def("Render", (void (Entity::*)(bool))&Entity::Render)
            .def("Render", (bool (Entity::*)()const)&Entity::Render)
            .def("IsUiElement", (void (Entity::*)(bool))&Entity::IsUiElement)
            .def("IsUiElement", (bool (Entity::*)()const)&Entity::IsUiElement)
	];
    return 1;
}