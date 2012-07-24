#include <vector>
#include <stdexcept>
#include <string>
#include "state.h"
#include "gamestate.h"
#include "menustate.h"
#include "statemanager.h"

std::vector<State*> StateManager::mStates;
int StateManager::mActiveID;

void StateManager::InitIntro(){
	mActiveID = -1;
	SetActiveState("intro");
}
void StateManager::Register(State *state){
	mStates.push_back(state);
}
int StateManager::IdFromName(std::string name){
	for (int i = 0; i < mStates.size(); ++i){
		if (mStates.at(i)->Name() == name)
			return i;
	}
	return -1;
}
void StateManager::SetActiveState(std::string name){
	int id = IdFromName(name);
	if (mActiveID == id)
		return;
	if (id == -1)
		throw std::runtime_error("Failed to find state");
	if (mActiveID != -1)
		mStates.at(mActiveID)->Free();
	//Save and quit the active state, the load and start the new state
	mStates.at(id)->Init();
	mStates.at(id)->Run();
}
void StateManager::LoadState(std::string name){
	
}
void StateManager::SaveState(std::string name){

}
