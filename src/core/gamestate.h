#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <memory>
#include "external/json/json.h"
#include "state.h"
#include "timer.h"
#include "map.h"

///A state for running a game level
/**
*  A simple game state that can run a level of the game
*/
class GameState : public State{
public:
	GameState();
	~GameState();
	/**
	*  Run the state, this function becomes the main Input thread
	*  after starting up the physics and rendering threads
	*  @return The next state to run, returning quit exits program
	*/
	std::string Run();
	/**
	*  Write the state data to a Json Value and return it so that
	*  it can be saved and loaded later
	*  @see State::Save for saving inherited members
	*  @return Json::Value containing the state's save data
	*/
	Json::Value Save();
	/**
	*  Load the state data from a json value
	*  @see State::Load for loading inherited members, besides the 
	*  manager as it must be loaded differently for each state type
	*  @param val The Json::Value containing the data to load
	*/
	void Load(Json::Value val);

protected:
	/**
	*  The state's rendering thread, takes care of drawing all objects
	*  and providing framerate limiting condition variable notifications
	*  to all other threads
	*  NOT USED AT THE MOMENT
	*/
	//void RenderThread();
	/**
	*  The state's physics thread, takes care of updating and moving
	*  all objects and managing physics between the objects
	*  NOT USED AT THE MOMENT
	*/
	//void PhysicsThread();
	///Initialize state memory
	void Init();
	///Free state memory
	void Free();

private:
	std::shared_ptr<Map> mMap;
	std::shared_ptr<TileSet> mTileSet;

};

#endif