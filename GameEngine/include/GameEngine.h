//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file GameEngine.h
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Highest level file, holds all needed includes. Create an instance of this object and call .run()
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Animation.h"
#include "Button.h"
#include "Camera.h"
#include "Display.h"
#include "Entity.h"
#include "Frame.h"
#include "GameObject.h"
#include "GameState.h"
#include "InputHandler.h"
#include "Parameters.h"
#include "Pixel.h"
#include "Position.h"
#include "Printable.h"
#include "PrintableFactory.h"
#include "RGB.h"
#include "Slider.h"
#include "Sprite.h"
#include "UIElement.h"
#include <chrono>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class GameEngine
///
/// Highest level class, holds all needed includes. Create an instance of this object and call .run()
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class GameEngine
{
private:
   GameState* currentState;

   void exit();

public:
   GameEngine(GameState* initialState) : currentState(initialState)
   {
      Display::initCurse();
      engineRunning = true;
      currentState->onEnter();
   }

   void run();
};

// Global InputHandler instance
extern InputHandler globalInputHandler;