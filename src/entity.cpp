#include <string>
#include <lua.hpp>
#include <luabind/luabind.hpp>
#include "math.h"
#include "entity.h"

Entity::Entity() : mMouseOver(false), mL(nullptr), mScript(""){
}
Entity::Entity(std::string script) : mMouseOver(false), mL(nullptr){
	OpenScript(script);
}
Entity::~Entity(){
	if (mL != nullptr)
		lua_close(mL);
}
void Entity::Init(){
	std::cout << "Calling Init" << std::endl;
	//We catch exceptions so that if the function doesn't exist the program 
	//won't crash. This lets us skip implementing functions we don't need
	//int scripts
	try{
		luabind::call_function<void>(mL, "Init", this);
	}
	catch(...){
		std::cout << "Init issue: " << lua_error(mL) << std::endl;
	}
}
void Entity::Update(){
	if (mL == nullptr)
		return;
	try{
		luabind::call_function<void>(mL, "Update");
	}
	catch(...){
	}
}
void Entity::Move(float deltaT){
	if (mL == nullptr)
		return;
	try{
		luabind::call_function<void>(mL, "Move", deltaT);
	}
	catch(...){
	}
}
void Entity::Draw(Camera *camera){
	if (mL == nullptr)
		return;
	try{
		luabind::call_function<void>(mL, "Draw", camera);
	}
	catch(...){
	}
}
void Entity::OnMouseDown(){
	if (mL == nullptr)
		return;
	try{
		luabind::call_function<void>(mL, "OnMouseDown");
	}
	catch(...){
	}
}
void Entity::OnMouseUp(){
	if (mL == nullptr)
		return;
	try{
		luabind::call_function<void>(mL, "OnMouseUp");
	}
	catch(...){
	}
}
void Entity::OnMouseEnter(){
	if (mL == nullptr)
		return;
	try{
		luabind::call_function<void>(mL, "OnMouseEnter");
	}
	catch(...){
	}
}
void Entity::OnMouseExit(){
	if (mL == nullptr)
		return;
	try{
		luabind::call_function<void>(mL, "OnMouseExit");
	}
	catch(...){
	}
}
void Entity::CheckMouseOver(const Vector2f &pos){
	//Only trigger OnMouseEnter if the mouse is colliding and wasn't before
	if (Math::CheckCollision(pos, mPhysics.Box()) && !mMouseOver){
		OnMouseEnter();
		mMouseOver = true;
	}
	//Only trigger mouse exit if the mouse was colliding, but isn't anymore
	else if (!Math::CheckCollision(pos, mPhysics.Box()) && mMouseOver){
		OnMouseExit();
		mMouseOver = false;
	}
}
bool Entity::GetMouseOver(){
	return mMouseOver;

}
void Entity::SetCollisionMap(CollisionMap map){
	mPhysics.SetMap(map);
}
void Entity::OpenScript(std::string script){
	mScript = script;
	if (mScript != ""){
		mL = lua_open();
		luaL_openlibs(mL);
		luabind::open(mL);
		//Perform lua module initialization here
		Entity::RegisterLua(mL);
		luaL_dofile(mL, mScript.c_str());
		std::cout << "Script opened" << std::endl;
	}
}
Rectf Entity::Box(){
	return mPhysics.Box();
}
void Entity::SetTag(std::string tag){
	mTag = tag;
}
std::string Entity::Tag(){
	return mTag;
}
Json::Value Entity::Save(){
	Json::Value val;
	val["image"]   = mImage.Save();
	val["physics"] = mPhysics.Save();
	val["tag"]	   = mTag;
	val["script"]  = mScript;
	return val;
}
void Entity::Load(Json::Value val){
	mPhysics.Load(val["physics"]);
	mImage.Load(val["image"]);
	mTag = val["tag"].asString();
	OpenScript(val["script"].asString());
	//Once migration to Lua is complete for entities, call Entity::Init here
}
void Entity::RegisterLua(lua_State *l){
	using namespace luabind;

	module(l, "LPC")[
		class_<Entity>("Entity")
			.def(constructor<>())
			.def(constructor<std::string>())
			.def("Init", &Entity::Init)
			.def("Update", &Entity::Update)
			.def("Move", &Entity::Move)
			.def("Draw", &Entity::Draw)
			.def("OnMouseDown", &Entity::OnMouseDown)
			.def("OnMouseUp", &Entity::OnMouseUp)
			.def("OnMouseEnter", &Entity::OnMouseEnter)
			.def("OnMouseExit", &Entity::OnMouseExit)
			.def("Box", &Entity::Box)
			.def("SetTag", &Entity::SetTag)
			.def("Tag", &Entity::Tag)
	];
}