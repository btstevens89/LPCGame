#include <array>
#include "SDL.h"
#include "SDL_image.h"
#include "rect.h"
#include "image.h"

#include <iostream>

Image::Image(const std::string file){
	mRect.w = 0;
	mRect.h = 0;
	mSurface = nullptr;
	mClips = nullptr;
	mNumClips = 0;
	LoadImage(file);
}
Image::Image(){
	mRect.w = 0;
	mRect.h = 0;
	mSurface = nullptr;
	mClips = nullptr;
	mNumClips = 0;
}
Image::~Image(){
	if (mSurface != nullptr)
		SDL_FreeSurface(mSurface);
	if (mClips != nullptr)
		delete[] mClips;
}
void Image::LoadImage(const std::string file){
	//if another image was already loaded, free that one
	if (mSurface != nullptr)
		SDL_FreeSurface(mSurface);
	SDL_Surface *loadedImage = nullptr;
	//load image
	loadedImage = IMG_Load(file.c_str());
	if (!loadedImage)
		throw std::runtime_error("LoadImage failed, could not load: " + file);
	//optimize the image
	mSurface = SDL_DisplayFormatAlpha(loadedImage);
	//free unoptimized image
	SDL_FreeSurface(loadedImage);
}
void Image::Move(Vector2f vect){
	mRect += vect;
}
SDL_Surface* Image::Surface(){
	return mSurface;
}
Rectf Image::Box(){
	return mRect;
}
Recti Image::Clip(int clipNum){
	if (clipNum > mNumClips || clipNum < 0 || mNumClips == 0)
		throw std::runtime_error("Clip num out of bounds");
	return mClips[clipNum];
}
void Image::SetPos(float x, float y){
	mRect.x = x;
	mRect.y = y;
}
void Image::SetPos(Vector2f vec){
	mRect.x = vec.x;
	mRect.y = vec.y;
}
void Image::SetClips(const std::vector<Recti> &clips){
	mNumClips = clips.size();
	mClips = new Recti[clips.size()];
	for (int i = 0; i < clips.size(); ++i)
		mClips[i] = clips.at(i);
}