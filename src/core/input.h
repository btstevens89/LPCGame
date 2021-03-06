#ifndef INPUT_H
#define INPUT_H

#include <memory>
#include <string>
#include <SDL.h>
#include "base.h"

///Enum for the 3 mouse buttons
enum MOUSE { LEFT = 1, MIDDLE = 2, RIGHT = 3 };

///Input handler wrapper
/**
*  A simple wrapper around SDL's input handling system
*/
class Input {
public:
    Input();
    ~Input();
    ///Initialize the input manager
    static void Init();
    ///Read event input
    static void PollEvent();
    //Functions for interacting with keyboard
    /**
    *  Check if a key is being pressed down
    *  @param keyCode The string of the key to check, ex: 'a'
    *  @return T if key is down
    */
    static bool KeyDown(std::string keyCode);
    /**
    *  Check if a key is being pressed down
    *  @param keyCode The SDL_SCANCODE of the key
    *  @return T if key is down
    */
    static bool KeyDown(int keyCode);
    //Functions for interacting with mouse
    /**
    *  Check if the mouse button is clicked
    *  @param button The button to check
    */
    static bool MouseClick(int button);
    /**
    *  Check if the mouse button is currently down
    *  @param button The button to check
    */
    static bool MouseDown(int button);
    ///Get the mouse button event
    static SDL_MouseButtonEvent GetClick();
    ///Check if the mouse moved
    static bool MouseMotion();
    ///Get the mouse motion
    static SDL_MouseMotionEvent GetMotion();
    ///Get the current mouse position
    static Vector2f MousePos();
    //Functions for interacting with Joystick
    /**
    *  Get the input level of the desired joystick axis
    *  input level value ranges from -1 to 1
    *  @param axis The axis to get value from
    *  @return The input level of the axis, from -1 to 1
    */
    static float GetJoyAxis(int axis);
    /**
    *  Check if a Joystick button is being pressed
    *  @param button The button to check
    *  @return T/F whether or not the button is being pushed
    */
    static bool GetJoyButton(int button);
    /**
    *  Check the position of the Joystick hat
    *  @param hat The hat to check position of
    *  @return The position of the hat
    */
    static int GetJoyHat(int hat);
    ///Check if a Joystick is available
    static bool JoystickAvailable();
    ///Check if the Joystick has haptic support (force feedback)
    static bool JoySupportsHaptic();
    ///Check if the program has been quit out of
    static bool Quit();
    ///Clear input data, used when changing states to clear old input
    static void Clear();
    ///Close the joystick and any other input items that need to be exited
    static void Close();

private:
    static void ClearQuit();
    ///Clear the keystates
    static void ClearKeys();
    ///Clear the mouse
    static void ClearMouse();

private:
    static SDL_Event evt;
    static bool mQuit, mMouseMove, mMouseClick;
    static Uint8 *mKeyStates;
    static SDL_Joystick *mJoystick;
    static SDL_MouseButtonEvent mButtonEvt;
    static SDL_MouseMotionEvent mMotionEvt;
};

#endif