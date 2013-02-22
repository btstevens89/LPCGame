#include "external/json/json.h"
#include "debug.h"
#include "state.h"

State::State() 
	: mManager(nullptr), mCamera(nullptr), mName(""), mExit(false), 
	  mExitCode("quit")//, mCondBool(false)
{
}
State::~State(){}
std::string State::Run(){
	return mExitCode;
}
void State::Init(){
    //Call the script
    if (!mScript.Open())
		return;

    mScript.FunctionInterface()->CallFunction<void>("Init");
}
void State::Free(){
    mScript.Close();
}
void State::LogicUpdate(){
    //Call the script
    if (!mScript.Open())
		return;

    mScript.FunctionInterface()->CallFunction<void>("LogicUpdate");
}
void State::RenderUpdate(){
    //Call the script
    if (!mScript.Open())
		return;

    mScript.FunctionInterface()->CallFunction<void>("RenderUpdate", std::weak_ptr<Camera>(mCamera));
}
void State::SetExit(std::string val){
	mExit = true;
	mExitCode = val;
}
void State::UnsetExit(){
	mExit = false;
	mExitCode = "quit";
}
std::shared_ptr<EntityManager> State::Manager(){
    return mManager;
}
void State::SetName(std::string name){
	mName = name;
}
std::string State::Name(){
	return mName;
}
Json::Value State::Save(){
	Json::Value val;
	val["entities"] = mManager->Save();
	val["name"]	    = mName;
	val["camera"]   = mCamera->Save();
    val["script"]   = mScript.File();

	return val;
}
void State::Load(Json::Value val){
	mName = val["name"].asString();
	mCamera->Load(val["camera"]);
    mScript.OpenScript(val["script"].asString());
}