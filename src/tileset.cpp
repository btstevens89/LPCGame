#include <array>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <SDL.h>
#include <luabind/luabind.hpp>
#include "../externals/json/json.h"
#include "rect.h"
#include "jsonhandler.h"
#include "tileset.h"
#include "window.h"

TileSet::TileSet()
{
}
TileSet::~TileSet(){
}
void TileSet::Load(Json::Value val){
	int size = val.size();
	if (size > 0){
		for (int i = 0; i < size; i++)
			Add(val[i]["file"].asString());
	}
}
void TileSet::Add(const std::string &file){
	if (file == "")
		return;

    //Add this image
	std::shared_ptr<SDL_Texture> newTexture;
    newTexture.reset(Window::LoadTexture(file), SDL_DestroyTexture);
    mImageSet[file] = newTexture;

    JsonHandler handler(file);
    //You may want to catch potential errors being thrown by read
    ParseImageJson(handler.Read(), file);
}
SDL_Texture* TileSet::Texture(const std::string &file){
	mTileSetMap::const_iterator it;
	it = mTileSet.find(file);
	if (it != mTileSet.end()){
		mImageSetMap::const_iterator iit;
		iit = mImageSet.find(it->second.Filename());
		if (iit != mImageSet.end())
			return (SDL_Texture*)iit->second.get();
	}
	return nullptr;
}
Recti TileSet::Clip(const std::string &file){
	mTileSetMap::const_iterator it;
	it = mTileSet.find(file);
	if (it != mTileSet.end())
		return it->second.Box();
	Recti nullRecti(0,0,0,0);
	return nullRecti;
}
bool TileSet::Solid(const std::string &file){
	mTileSetMap::const_iterator it;
	it = mTileSet.find(file);
	if (it != mTileSet.end())
		return it->second.Solid();
	return false;
}
void TileSet::ParseImageJson(Json::Value val, const std::string &file){
	unsigned int a = file.rfind('.');
	unsigned int b = file.rfind('/');
	std::string fileName = file.substr(b+1,a-b-1);

	if (val.isMember("clips")){
		int size = val["clips"].size();
		for (int i = 0; i < size; ++i){
			Json::Value tile = val["clips"][i];

			//Create new tile
			Tile newTile(tile);
			newTile.SetFilename(file);
			
			//Add tile to tileset
			std::string tileSetEntry = fileName + " - " + tile["name"].asString();
			mTileSet[tileSetEntry] = newTile;

		}
	}
}
Json::Value TileSet::Save() {
	Json::Value val;
	
	int i = 0;
	for(mImageSetMap::iterator it = mImageSet.begin(); it != mImageSet.end(); it++)
	{
		val[i]["file"] = it->first;
		i++;
	}
	return val;
}
