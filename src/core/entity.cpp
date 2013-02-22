#include <string>
#include <lua.hpp>
#include <fstream>
#include "luac/luacentity.h"
#include "math.h"
#include "luascript.h"
#include "jsonhandler.h"
#include "debug.h"
#include "entity.h"

Entity::Entity() : mPhysics(std::make_shared<Physics>()), mName(""), mTag(""), mConfigFile(""), 
    mMouseOver(false), mClicked(false), mRender(true), mUiElement(false), mScript(std::make_shared<LuaScript>())
{
}
Entity::Entity(std::string file) : mPhysics(std::make_shared<Physics>()), mName(""), mTag(""), mConfigFile(""),
    mMouseOver(false), mClicked(false), mRender(true), mUiElement(false), mScript(std::make_shared<LuaScript>())
{
    Load(file);
}
Entity::~Entity(){
}
void Entity::Init(std::shared_ptr<Entity> self){
	//We catch exceptions so that if the function doesn't exist the program 
	//won't crash. This lets us skip implementing functions we don't need
	//int scripts
	if (!mScript->Open())
		return;
    
    //The Entity's table will be the global table
    //with the entity's name
    if (self != nullptr){
        //Push the self pointer onto the state so the script can use it
        LuaC::EntityLib::Push(mScript->Get(), self, "entity");
    }
    mScript->FunctionInterface()->CallFunction<void>("Init");
}
void Entity::Free(){
    mScript->Close();
}
void Entity::Update(){
    //Should deltaT be passed to update instead?
	if (!mScript->Open())
		return;
	
    mScript->FunctionInterface()->CallFunction<void>("Update");
}
void Entity::Move(float deltaT){
    //Shouldn't we call Physics::Move here?
	if (!mScript->Open())
		return;

    mScript->FunctionInterface()->CallFunction<void>("Move", deltaT);
    //Move the object
    mPhysics->Move(deltaT);
}
void Entity::Draw(std::weak_ptr<Camera> camera){
    //Draw entity
	if (!mScript->Open())
		return;
    //Shouldn't i be drawing the base image here though, instead
    //of making the script have to even draw the basic entity image?
    mScript->FunctionInterface()->CallFunction<void>("Draw", camera);
}
void Entity::OnMouseDown(){
    mClicked = true;

	if (!mScript->Open())
		return;
    mScript->FunctionInterface()->CallFunction<void>("OnMouseDown");
}
void Entity::OnMouseUp(){
    //We have to do this here for now b/c ObjectButtons don't have 
    //an attached script, however I'd prefer to call OnClick after OnMouseUp
    if (mClicked)
        OnClick();
    mClicked = false;
	
    if (!mScript->Open())
		return;
    mScript->FunctionInterface()->CallFunction<void>("OnMouseUp");
}
void Entity::OnClick(){
    if (!mScript->Open())
        return;
    mScript->FunctionInterface()->CallFunction<void>("OnClick");
}
void Entity::OnMouseEnter(){
    mMouseOver = true;
	if (!mScript->Open())
		return;
    mScript->FunctionInterface()->CallFunction<void>("OnMouseEnter");
}
void Entity::OnMouseExit(){
    mClicked = false;
    mMouseOver = false;

    if (!mScript->Open())
        return;
    mScript->FunctionInterface()->CallFunction<void>("OnMouseExit");
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
std::shared_ptr<LuaScript> Entity::Script(){
    return mScript;
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
	    val["script"]  = mScript->File();
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
	val["script"]  = mScript->File();
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
	mScript->OpenScript(val["script"].asString());
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