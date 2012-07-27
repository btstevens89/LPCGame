#ifndef CAMERA_H
#define CAMERA_H

#include <memory>
#include "window.h"
#include "rect.h"
#include "gameobject.h"

/*
*  A simple camera that can be given a gameobject to focus on
*  and follow, or can be moved manually
*/
class Camera{
public:
	Camera();
	~Camera();
	/*
	*  Register the gameobject to focus on with the camera
	*  @param obj: the object for the camera to follow/center on
	*/
	void SetFocus(std::shared_ptr<GameObject> obj);
	//Update the camera's position to keep the object centered
	void Update();
	/*
	*  Check if an Rect is in the camera
	*  @param box: the box to check if it's in the camera
	*  @returns: True if object is in camera
	*/
	bool InCamera(Rectf box) const;
	//Setters & Getters
	void SetBox(Rectf box);
	Rectf Box() const;
	//Returns the offset to apply to objects that should be scrolling
	Vector2f Offset() const;

private:
	std::weak_ptr<GameObject> mFocus;
	Rectf mBox;
};

#endif