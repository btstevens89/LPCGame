#include <stdexcept>
#include <thread>
#include <mutex>
#include "map.h"
#include "window.h"
#include "gameobjectmanager.h"
#include "timer.h"
#include "player.h"
#include "npc.h"
#include "input.h"
#include "button.h"
#include "menustate.h"
#include "gamestate.h"
#include "statemanager.h"

#include "debugger.h"
#include <iostream>

int main(int argc, char** argv){
	//Initialize statics
	Window::Init();
	Input::Init();

	//TODO: What if i wrote wrappers around texture and such that would call
	//the destroy when its destructor was called, then used smart pointers?
	//I think the Image class should hold a shared pointer and return weak pointers
	//when the texture pointer is requested

	MenuState *menuState = new MenuState();
	menuState->SetName("intro");
	StateManager::Register((State*)menuState);

	GameState *gameState = new GameState();
	gameState->SetName("game");
	StateManager::Register((State*)gameState);

	StateManager::InitIntro();
	//menuState->Run();
	//delete menuState;
	//Window::Quit();
	//return 0;


	//gameState->Run();
	//delete gameState;

/*
	//Setup our game stuff
	Timer delta;
	Map *map		= new Map();
	Player *player	= new Player();
	GameObjectManager *manager = new GameObjectManager();

	//Register the objects with the manager
	player->Start(10, 10);
	manager->Register((GameObject*)player);

	//Setup initial collision maps
	manager->SetCollisionMaps(map);
	//Register the manager with input
	Input::RegisterManager(manager);

	delta.Start();
	while (!Input::Quit()){
		//EVENT POLLING
		Input::PollEvent();

		///LOGIC
		manager->Update();
		manager->SetCollisionMaps(map);

		float deltaT = delta.GetTicks() / 1000.f;
		manager->Move(deltaT);

		delta.Start();

		///RENDERING
		Window::Clear();
		map->Draw();
		manager->Draw();

		//refresh window
		Window::Present();
	}
	delete map;
	delete manager;
*/
	Window::Quit();

	return 0;
}