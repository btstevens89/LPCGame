#ifndef STATE_H
#define STATE_H

#include <string>
#include "json/json.h"
#include "gameobjectmanager.h"

/*
*  The base class for our state machine, provides functions
*  for initializing state, running the state and memory clean-up
*/
class State{
public:
	State();
	virtual ~State();
	//Initialize state memory
	virtual void Init() = 0;
	//Run the state
	virtual std::string Run() = 0;
	//Set the exit code for Run, and set mExit to true
	void SetExit(std::string val);
	//Reset the exit values to default
	void UnsetExit();
	//Free the memory used by the state
	virtual void Free() = 0;
	/*
	*  Save the state data so that it can be loaded later
	*/
	virtual void Save() = 0;
	//Setters & Getters
	void SetName(std::string name);
	std::string Name();

protected:
	GameObjectManager *mManager;
	std::string mName;
	bool mExit;
	std::string mExitCode;
};


#endif