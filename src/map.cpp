#include <set>
#include <vector>
#include <string>
#include <cmath>
#include "../externals/json/json.h"
#include "window.h"
#include "image.h"
#include "tile.h"
#include "camera.h"
#include "map.h"

Map::Map(){
}
Map::~Map(){
	mTiles.clear();
}
void Map::Draw(Camera *cam){
	//Use the camera box to get the indices of all the tiles in visible in camera
	if (cam != nullptr){
		std::set<int> indices = CalculateIndex(cam->Box());
		for (int i : indices){
			if (i < mTiles.size()){
				Rectf pos = Math::FromSceneSpace(cam, mTiles.at(i).Box());
				Window::Draw(&mImage, pos,
					&(Recti)mImage.Clip(mTiles.at(i).Type()));
			}
		}
	}
	//If no camera we default to drawing all tiles
	else
		for (int i = 0; i < mTiles.size(); ++i){
			Window::Draw(&mImage, mTiles.at(i).Box(), 
				&(Recti)mImage.Clip(mTiles.at(i).Type()));
		}
}
Json::Value Map::Save(){
	Json::Value map;
	//Save the map width and height
	map["mBox"]["w"] = mBox.w;
	map["mBox"]["h"] = mBox.h;
	map["image"] = mImage.Save();
	//Save the tiles
	for (int i = 0; i < mTiles.size(); ++i){
		map["tiles"][i] = mTiles.at(i).Save();
	}
	
	return map;
}
void Map::Load(Json::Value val){
	mBox.Set(0, 0, val["mBox"]["w"].asInt(), val["mBox"]["h"].asInt());
	mImage.Load(val["image"]);

	//Load the tiles
	Json::Value tiles = val["tiles"];
	for (int i = 0; i < tiles.size(); ++i){
		Tile tempTile;
		tempTile.Load(tiles[i]);
		mTiles.push_back(tempTile);
	}
}
void Map::GenerateStressMap(Json::Value val){
	int numTiles = val["numTiles"].asInt();
	mImage.Load(val["image"]);
	mBox.Set(0, 0, Window::Box().w, Window::Box().h);
	//Determine the tile w/h to fill the window with numTiles
	int tileSize = sqrtf((Window::Box().w * Window::Box().h) / numTiles);
	//Generate the map
	int tPerCol = Window::Box().w / tileSize;
	int col = 0;
	for (int i = 0; i < numTiles; ++i){
		if (i != 0 && i % tPerCol == 0)
			++col;
		Recti tRect(col * tileSize, i % tPerCol * tileSize, tileSize, tileSize);
		Tile tempTile;
		tempTile.SetBox(tRect);
		tempTile.SetSolid(false);
		tempTile.SetType(0);
		mTiles.push_back(tempTile);
	}
}
int Map::CalculateIndex(int x, int y, int w, int h) const{
	//if it's in bounds calculate the index
	if ((x > 0 && x < w) && (y > 0 && y < h)){
		return (x / TILE_WIDTH + (y / TILE_HEIGHT) * (w / TILE_WIDTH)); 
	}
	else{ 
		return -1;
	}
}

std::set<int> Map::CalculateIndex(Recti area) const{
	std::set<int> tileIndices;
	//TODO: How can this be done without all the for loops?
	//run through the area

	//generating these beforehand so that there does not
	//have to be so many calls during the future loops
	int area_x = area.X();
	int area_y = area.Y();
	int area_w = area.W();
	int area_h = area.H();
	int mbox_w = mBox.W();
	int mbox_h = mBox.H();

	for (int y = area_y; y <= area_y + area_h; y += TILE_HEIGHT / 2){
		for (int x = area_x; x <= area_x + area_w; x += TILE_WIDTH / 2){

			//find the appropriate index and place it with the tiles
			int index = CalculateIndex(x, y, mbox_w, mbox_h);
			if (index >= 0){
				tileIndices.insert(index);
			}
		}
	}

	if (tileIndices.size() != 0)
		return tileIndices;
	else
		throw std::runtime_error("Invalid area");
}
CollisionMap Map::GetCollisionMap(const Recti &target, int distance){
	//get the indices of the desired tiles
	Recti area(target.X() - distance * TILE_WIDTH, target.Y() - distance * TILE_HEIGHT,
		((target.X() + target.W() + distance * TILE_WIDTH) - (target.X() - distance * TILE_WIDTH)),
		((target.Y() + target.H() + distance * TILE_HEIGHT) - (target.Y() - distance * TILE_HEIGHT)));

	std::set<int> indices;
	try{
		indices = CalculateIndex(area);
	}
	catch (...){
	}
	//Setup the collision map
	CollisionMap localMap;
	for (int i : indices){
		if (i < mTiles.size() && mTiles.at(i).Solid())
			localMap.push_back(mTiles.at(i).Box());
	}
	return localMap;
}
Recti Map::Box() const{
	return mBox;
}