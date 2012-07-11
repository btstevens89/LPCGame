#ifndef GAMEOBJECTMANAGER_H
#define GAMEOBJECTMANAGER_H

#include <vector>
#include "gameobject.h"
#include "map.h"

//Typedef for a vector of GameObject pointers
typedef std::vector<GameObject*> GameObjectList;

/*
 * A class to simplify handling of the objects, and simple
 * collision interactions between them
*/
class GameObjectManager{
public:
	GameObjectManager();
	~GameObjectManager();
	///Draw the game objects
	void Draw();
	/*
	*	Move the game objects
	*	@param deltaT: the time elapsed
	*/
	void Move(float deltaT);
	/*
	*	Setup object collision maps
	*	@param map: the map to get the collision maps from
	*/
	void SetCollisionMaps(Map *map);
	/*
	*	Add a gameobject pointer to the list
	*	@param *obj: the object pointer to add
	*/
	void Register(GameObject *obj);

private:
	//Maybe i can have a function that returns a collision map of local entities
	//and simply add that collision map to the local collision map of the tiles
	CollisionMap GetEntityCollisionMap(const Rectf &target, int distance = 2 * TILE_WIDTH);

private:
	GameObjectList mGameObjects;
};

#endif