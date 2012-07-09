#include <cmath>
#include <sstream>
#include "SDL.h"
#include "gameobject.h"
#include "rect.h"
#include "vectors.h"
#include "math.h"
#include "physics.h"
#include "window.h"
#include "image.h"
#include "npc.h"
#include "player.h"


Npc::Npc(){
	//initialize box
	Rectf box(10, 10, PLAYER_WIDTH, PLAYER_HEIGHT);
	mPhysics.SetBox(box);
	PhysicalConstants physConst;
	physConst.hSpeed	= PLAYER_HSPEED;
	physConst.hAccel	= PLAYER_HACCEL;
	mPhysics.SetPhysConstants(physConst);

	mImage.LoadImage("images/image.png");
}
Npc::~Npc(){
}
void Npc::Move(float deltaT){
	mPhysics.Move(deltaT);
}
void Npc::Draw(){
	Window::Draw(&mImage, (SDL_Rect)mPhysics.Box());
}
void Npc::SetMove(int move){
	if (move == Math::LEFT || move == Math::RIGHT)
		mPhysics.SetHorizDir(move);
	else if (move == Math::UP || move == Math::DOWN)
		mPhysics.SetVertDir(move);
	else if (move == Physics::MOVE::STOP){
		mPhysics.SetHorizDir(move);
		mPhysics.SetVertDir(move);
	}
}