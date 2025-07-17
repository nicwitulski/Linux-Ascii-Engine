//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file AppState.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of AppState for main animation editor functionality
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "AppState.h"
#include "QuitState.h"
#include "../../../GameEngine/include/Display.h"
#include "../../../GameEngine/include/Frame.h"
#include <ncurses.h>
#include <algorithm>

// public ----------------------------------------------------------------------------------------------------
void AppState::onEnter()
{
   currentCamera = std::make_shared<Camera>(SCREEN_LENGTH, SCREEN_HEIGHT);

   Frame              currentFrame = Frame(Sprite(), 10);
   std::vector<Frame> allFrames;
   allFrames.push_back(currentFrame);
   Animation drawingAnimation = Animation("newAnimation", allFrames, true);
   drawingAnimation.setPlaying(false);
   std::vector<Animation> allAnimations;
   allAnimations.push_back(drawingAnimation);

   drawnEntity = std::make_shared<Entity>("newEntity", allAnimations, true, true);
   drawnEntity->getCurrentAnimationMutable().setRepeats(true);
   printablesToSave.push_back(drawnEntity);

   visibleEntity = std::make_shared<Entity>("newEntity", allAnimations, true, true);
   visibleEntity->getCurrentAnimationMutable().setRepeats(true);
   ncursesWindows.at(0)->addPrintable(visibleEntity);

   playerEntity = visibleEntity;

   // Create greyed background entity at a lower layer
   greyedBackgroundEntity = std::make_shared<Entity>("greyedBackground", allAnimations, false, true);
   greyedBackgroundEntity->setAllAnimationSpriteLayers(-1); // Lower layer than main entity
   ncursesWindows.at(0)->addPrintable(greyedBackgroundEntity);

   // Initialize utility classes
   frameManager.setEntities(visibleEntity, drawnEntity, greyedBackgroundEntity);

   selectNewCharacter = false;

   // Create buttons
   currentColorsButton = PrintableFactory::newButton("Current Colors: x\nClick to Edit",
                                                     &AppState::currentColorsButtonFunc, this);
   currentCharacterButton =
         PrintableFactory::newButton("Current Character: x", &AppState::currentCharacterButtonFunc, this);
   eraserSelectButton = PrintableFactory::newButton("Eraser", &AppState::eraserSelectButtonFunc, this);
   brushSizeButton =
         PrintableFactory::newButton("Brush size: 1 characters", &AppState::brushSizeButtonFunc, this);
   frameLengthButton = PrintableFactory::newButton("Frame Length", &AppState::frameLengthButtonFunc, this);
   nextFrameButton   = PrintableFactory::newButton("Next Frame", &AppState::nextFrameButtonFunc, this);
   playAnimationButton =
         PrintableFactory::newButton("Play Animation", &AppState::playAnimationButtonFunc, this);
   previousFrameButton =
         PrintableFactory::newButton("Previous Frame", &AppState::previousFrameButtonFunc, this);
   quitButton = PrintableFactory::newButton("Quit", &AppState::quitButtonFunc, this);

   // Create frame duration slider
   frameDurationSlider = std::make_shared<Slider>(20, true); // 20 positions, horizontal
   frameDurationSlider->setDynamicPosition(ScreenLockPosition::BOTTOM_LEFT_CORNER, StackDirection::VERTICAL);
   frameDurationSlider->setVisability(true);
   frameDurationSlider->setMoveableByCamera(false);
   frameDurationSlider->setAllAnimationSpriteLayers(1);

   // Add slider to the window
   if (!ncursesWindows.empty())
   {
      ncursesWindows.at(0)->addPrintable(frameDurationSlider);
   }

   // Create brush size slider
   brushSizeSlider = std::make_shared<Slider>(6, true); // 6 positions (1-6), horizontal
   brushSizeSlider->setVisability(true);
   brushSizeSlider->setMoveableByCamera(false);
   brushSizeSlider->setAllAnimationSpriteLayers(1);

   // Add slider to the window
   if (!ncursesWindows.empty())
   {
      ncursesWindows.at(0)->addPrintable(brushSizeSlider);
   }

   currentCharacterButton->setDynamicPosition(ScreenLockPosition::TOP_RIGHT_CORNER);
   eraserSelectButton->setDynamicPosition(ScreenLockPosition::TOP_RIGHT_CORNER);
   brushSizeButton->setDynamicPosition(ScreenLockPosition::TOP_RIGHT_CORNER);
   brushSizeSlider->setDynamicPosition(ScreenLockPosition::TOP_RIGHT_CORNER, StackDirection::VERTICAL);

   frameLengthButton->setDynamicPosition(ScreenLockPosition::BOTTOM_LEFT_CORNER);

   currentColorsButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE);
   previousFrameButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE, StackDirection::HORIZONTAL);
   nextFrameButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE, StackDirection::HORIZONTAL);
   playAnimationButton->setDynamicPosition(ScreenLockPosition::BOTTOM_MIDDLE, StackDirection::HORIZONTAL);

   quitButton->setDynamicPosition(ScreenLockPosition::TOP_LEFT_CORNER);
   UIElement::updateAllLockedPositions();

   // Initialize button states
   updateButtonStates();

   // Initially hide the greyed background
   frameManager.clearGreyedBackground();

   // Initialize button highlighting state
   currentHoveredButton  = nullptr;
   currentClickedButton  = nullptr;
   currentSelectedButton = nullptr;

   // Initialize frame duration editing state
   editingFrameDuration = false;
   frameDurationInput   = "";

   // Initialize colors
   currentBackgroundColor = RGB(0, 0, 0);          // Black background
   currentTextColor       = RGB(1000, 1000, 1000); // White text
   colorEditWindowOpen    = false;
   colorEditWindow        = nullptr;

   // Set the initial colors on the drawing tool
   drawingTool.setTextColor(currentTextColor);
   drawingTool.setBackgroundColor(currentBackgroundColor);

   // Set initial selection to currentCharacterButton since user starts drawing with 'x'
   currentSelectedButton = currentCharacterButton;
   currentCharacterButton->highlight(RGB(250, 250, 250));

   // Update slider and button text based on current frame duration
   updateSliderFromFrameDuration();
   updateFrameDurationButtonText();

   // Update brush size slider and button text
   updateBrushSizeButtonText();

   // Update colors button text
   updateCurrentColorsButtonText();
}

// public ----------------------------------------------------------------------------------------------------
void AppState::update()
{
   if (selectNewCharacter)
   {
      // Only process printable characters, not special keys like KEY_MOUSE
      // Also prevent character selection during playback
      if (userInput >= 32 && userInput <= 126 && !visibleEntity->getCurrentAnimation().isPlaying())
      {
         drawingTool.setDrawingCharacter(static_cast<char>(userInput));
         selectNewCharacter = false;

         // Update button text to show the new character
         std::string buttonText = "Current Character: " + std::string(1, drawingTool.getDrawingCharacter());
         currentCharacterButton->setText(buttonText);

         // Keep the currentCharacterButton highlighted to show it's selected
         currentCharacterButton->highlight(RGB(250, 250, 250));

         // Update color display with new character
         updateCurrentColorsButtonText();

         // Refresh all locked positions after text change
         UIElement::updateAllLockedPositions();
      }
      // Don't process other inputs while waiting for character selection
   }
   else if (editingFrameDuration)
   {
      // Handle frame duration input (numbers, decimals, enter, escape)
      if (userInput == '\n' || userInput == '\r' || userInput == KEY_ENTER) // Enter key
      {
         // Try to parse the input as a float
         try
         {
            float newDuration = std::stof(frameDurationInput);
            // Clamp to valid range (0.1 to 5.0 seconds)
            newDuration = std::max(0.1f, std::min(5.0f, newDuration));

            // Update the current frame duration
            size_t currentIndex = visibleEntity->getCurrentAnimation().getCurrentFrameIndex();

            // Create new frames with updated duration
            Frame& currentVisibleFrame =
                  visibleEntity->getCurrentAnimationMutable().getFrameAtIndexMutable(currentIndex);
            Frame& currentDrawnFrame =
                  drawnEntity->getCurrentAnimationMutable().getFrameAtIndexMutable(currentIndex);

            Frame newVisibleFrame(currentVisibleFrame.getSprite(), newDuration);
            Frame newDrawnFrame(currentDrawnFrame.getSprite(), newDuration);

            // Replace the frames
            visibleEntity->getCurrentAnimationMutable().getFrameAtIndexMutable(currentIndex) = newVisibleFrame;
            drawnEntity->getCurrentAnimationMutable().getFrameAtIndexMutable(currentIndex) = newDrawnFrame;

            // Update slider position and button text
            updateSliderFromFrameDuration();
            updateFrameDurationButtonText();
         }
         catch (const std::exception&)
         {
            // Invalid input, just update button text to current value
            updateFrameDurationButtonText();
         }

         // Exit editing mode
         editingFrameDuration = false;
         frameDurationInput   = "";

         // Unhighlight the button
         frameLengthButton->unhighlight();
         currentSelectedButton = nullptr;
      }
      else if (userInput == 27) // Escape key
      {
         // Cancel editing
         editingFrameDuration = false;
         frameDurationInput   = "";

         // Unhighlight the button and restore original text
         frameLengthButton->unhighlight();
         currentSelectedButton = nullptr;
         updateFrameDurationButtonText();
      }
      else if ((userInput >= '0' && userInput <= '9') || userInput == '.')
      {
         // Add valid characters to input
         frameDurationInput += static_cast<char>(userInput);

         // Update button text to show current input
         std::string buttonText = "Duration: " + frameDurationInput;
         frameLengthButton->setText(buttonText);
         UIElement::updateAllLockedPositions();
      }
      else if (userInput == 8 || userInput == 127) // Backspace
      {
         // Remove last character
         if (!frameDurationInput.empty())
         {
            frameDurationInput.pop_back();
            std::string buttonText = "Duration: " + frameDurationInput;
            frameLengthButton->setText(buttonText);
            UIElement::updateAllLockedPositions();
         }
      }
   }
   else if (userInput == KEY_MOUSE)
   {
      MEVENT* mouseEvent = globalInputHandler.getLastMouseEvent();
      if (mouseEvent != nullptr)
      {
         Position mousePos(mouseEvent->x, mouseEvent->y);

         // Update button highlighting based on mouse position and state
         bool isMousePressed = (mouseEvent->bstate & BUTTON1_PRESSED) != 0;
         updateButtonHighlighting(mousePos, isMousePressed);

         if (mouseEvent->bstate & BUTTON2_PRESSED)
         {
            mouseHandler.startCameraDrag(mousePos);
         }
         if (mouseEvent->bstate & BUTTON1_PRESSED)
         {
            mouseHandler.startDrawing();

            // Check if mouse is over the sliders first
            if (frameDurationSlider->mouseInBounds(mousePos) &&
                !visibleEntity->getCurrentAnimation().isPlaying())
            {
               frameDurationSlider->setPositionFromMouse(mousePos);
               updateFrameDurationFromSlider();
               updateFrameDurationButtonText();
            }
            else if (brushSizeSlider->mouseInBounds(mousePos) &&
                     !visibleEntity->getCurrentAnimation().isPlaying())
            {
               brushSizeSlider->setPositionFromMouse(mousePos);
               updateBrushSizeFromSlider();
               updateBrushSizeButtonText();
            }
            else if (colorEditWindowOpen)
            {
               // Handle color edit window sliders
               bool handledByColorWindow = false;
               if (backgroundRedSlider->mouseInBounds(mousePos))
               {
                  backgroundRedSlider->setPositionFromMouse(mousePos);
                  updateColorsFromSliders();
                  handledByColorWindow = true;
               }
               else if (backgroundGreenSlider->mouseInBounds(mousePos))
               {
                  backgroundGreenSlider->setPositionFromMouse(mousePos);
                  updateColorsFromSliders();
                  handledByColorWindow = true;
               }
               else if (backgroundBlueSlider->mouseInBounds(mousePos))
               {
                  backgroundBlueSlider->setPositionFromMouse(mousePos);
                  updateColorsFromSliders();
                  handledByColorWindow = true;
               }
               else if (textRedSlider->mouseInBounds(mousePos))
               {
                  textRedSlider->setPositionFromMouse(mousePos);
                  updateColorsFromSliders();
                  handledByColorWindow = true;
               }
               else if (textGreenSlider->mouseInBounds(mousePos))
               {
                  textGreenSlider->setPositionFromMouse(mousePos);
                  updateColorsFromSliders();
                  handledByColorWindow = true;
               }
               else if (textBlueSlider->mouseInBounds(mousePos))
               {
                  textBlueSlider->setPositionFromMouse(mousePos);
                  updateColorsFromSliders();
                  handledByColorWindow = true;
               }

               // If not handled by color window and mouse is not over the color window itself, allow normal drawing
               if (!handledByColorWindow && !colorEditWindow->isMouseInWindow(mousePos) &&
                   !globalInputHandler.isMouseOverUI(mousePos) &&
                   !visibleEntity->getCurrentAnimation().isPlaying())
               {
                  int worldX = mouseEvent->x - currentCamera->getLengthOffset();
                  int worldY = mouseEvent->y - currentCamera->getHeightOffset();

                  if (drawingTool.isErasing())
                  {
                     // Erasing: use brush size
                     drawingTool.eraseAtPosition(visibleEntity, worldX, worldY);
                  }
                  else
                  {
                     // Drawing: use brush size
                     drawingTool.drawAtPosition(visibleEntity, worldX, worldY);
                  }
                  updateButtonStates();
               }
            }
            else if (!globalInputHandler.isMouseOverUI(mousePos) &&
                     !visibleEntity->getCurrentAnimation().isPlaying())
            {
               int worldX = mouseEvent->x - currentCamera->getLengthOffset();
               int worldY = mouseEvent->y - currentCamera->getHeightOffset();

               if (drawingTool.isErasing())
               {
                  // Erasing: use brush size
                  drawingTool.eraseAtPosition(visibleEntity, worldX, worldY);
               }
               else
               {
                  // Drawing: use brush size
                  drawingTool.drawAtPosition(visibleEntity, worldX, worldY);
               }
               updateButtonStates();
            }
         }

         if (mouseEvent->bstate & BUTTON2_RELEASED)
         {
            mouseHandler.stopCameraDrag();
         }
         if (mouseEvent->bstate & BUTTON1_RELEASED)
         {
            mouseHandler.stopDrawing();

            // Handle button release highlighting
            if (currentClickedButton)
            {
               // Only unhighlight if it's not a selected button
               if (currentClickedButton != currentSelectedButton)
               {
                  currentClickedButton->unhighlight();
                  // Re-highlight if mouse is still over the button
                  if (currentHoveredButton == currentClickedButton)
                  {
                     currentHoveredButton->highlight(RGB(750, 750, 750));
                  }
               }
               currentClickedButton = nullptr;
            }
         }

         if (mouseEvent->bstate & (BUTTON1_RELEASED | BUTTON1_CLICKED))
         {
            mouseHandler.stopDrawing();
         }
         if (mouseEvent->bstate & (BUTTON2_RELEASED | BUTTON2_CLICKED))
         {
            mouseHandler.stopCameraDrag();
         }

         if (mouseEvent->bstate & REPORT_MOUSE_POSITION)
         {
            // Update button highlighting for mouse movement
            updateButtonHighlighting(mousePos, false);

            if (mouseHandler.isCameraDragging())
            {
               Position currentMousePos(mouseEvent->x, mouseEvent->y);
               mouseHandler.updateCameraDrag(currentMousePos);
            }
            else if (mouseHandler.isDrawing())
            {
               // Check if mouse is over the sliders during drag
               if (frameDurationSlider->mouseInBounds(mousePos) &&
                   !visibleEntity->getCurrentAnimation().isPlaying())
               {
                  frameDurationSlider->setPositionFromMouse(mousePos);
                  updateFrameDurationFromSlider();
                  updateFrameDurationButtonText();
               }
               else if (brushSizeSlider->mouseInBounds(mousePos) &&
                        !visibleEntity->getCurrentAnimation().isPlaying())
               {
                  brushSizeSlider->setPositionFromMouse(mousePos);
                  updateBrushSizeFromSlider();
                  updateBrushSizeButtonText();
               }
               else if (colorEditWindowOpen)
               {
                  // Handle color edit window sliders during drag
                  bool handledByColorWindow = false;
                  if (backgroundRedSlider->mouseInBounds(mousePos))
                  {
                     backgroundRedSlider->setPositionFromMouse(mousePos);
                     updateColorsFromSliders();
                     handledByColorWindow = true;
                  }
                  else if (backgroundGreenSlider->mouseInBounds(mousePos))
                  {
                     backgroundGreenSlider->setPositionFromMouse(mousePos);
                     updateColorsFromSliders();
                     handledByColorWindow = true;
                  }
                  else if (backgroundBlueSlider->mouseInBounds(mousePos))
                  {
                     backgroundBlueSlider->setPositionFromMouse(mousePos);
                     updateColorsFromSliders();
                     handledByColorWindow = true;
                  }
                  else if (textRedSlider->mouseInBounds(mousePos))
                  {
                     textRedSlider->setPositionFromMouse(mousePos);
                     updateColorsFromSliders();
                     handledByColorWindow = true;
                  }
                  else if (textGreenSlider->mouseInBounds(mousePos))
                  {
                     textGreenSlider->setPositionFromMouse(mousePos);
                     updateColorsFromSliders();
                     handledByColorWindow = true;
                  }
                  else if (textBlueSlider->mouseInBounds(mousePos))
                  {
                     textBlueSlider->setPositionFromMouse(mousePos);
                     updateColorsFromSliders();
                     handledByColorWindow = true;
                  }

                  // If not handled by color window and mouse is not over the color window itself, allow normal drawing
                  if (!handledByColorWindow && !colorEditWindow->isMouseInWindow(mousePos) &&
                      !globalInputHandler.isMouseOverUI(mousePos) &&
                      !visibleEntity->getCurrentAnimation().isPlaying())
                  {
                     int worldX = mouseEvent->x - currentCamera->getLengthOffset();
                     int worldY = mouseEvent->y - currentCamera->getHeightOffset();

                     if (drawingTool.isErasing())
                     {
                        // Erasing: use brush size
                        drawingTool.eraseAtPosition(visibleEntity, worldX, worldY);
                        // Force display refresh for immediate visual feedback
                        displayNeedsCleared = true;
                     }
                     else
                     {
                        // Drawing: use brush size
                        drawingTool.drawAtPosition(visibleEntity, worldX, worldY);
                     }
                     updateButtonStates();
                  }
               }
               else if (!globalInputHandler.isMouseOverUI(mousePos) &&
                        !visibleEntity->getCurrentAnimation().isPlaying())
               {
                  int worldX = mouseEvent->x - currentCamera->getLengthOffset();
                  int worldY = mouseEvent->y - currentCamera->getHeightOffset();

                  if (drawingTool.isErasing())
                  {
                     // Erasing: use brush size
                     drawingTool.eraseAtPosition(visibleEntity, worldX, worldY);
                     // Force display refresh for immediate visual feedback
                     displayNeedsCleared = true;
                  }
                  else
                  {
                     // Drawing: use brush size
                     drawingTool.drawAtPosition(visibleEntity, worldX, worldY);
                  }
                  updateButtonStates();
               }
            }
         }
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
void AppState::onExit()
{
   clear();
   ncursesWindows.at(0)->clearPrintables();
   globalInputHandler.clear();

   // Clean up button highlighting state
   if (currentHoveredButton)
   {
      currentHoveredButton->unhighlight();
      currentHoveredButton = nullptr;
   }
   if (currentClickedButton)
   {
      currentClickedButton->unhighlight();
      currentClickedButton = nullptr;
   }
   if (currentSelectedButton)
   {
      currentSelectedButton->unhighlight();
      currentSelectedButton = nullptr;
   }

   // Clean up frame duration editing state
   if (editingFrameDuration)
   {
      editingFrameDuration = false;
      frameDurationInput   = "";
   }

   // Clean up color edit window
   if (colorEditWindowOpen)
   {
      closeColorEditWindow();
   }

   // Clean up greyed background entity
   greyedBackgroundEntity = nullptr;
}

// public ----------------------------------------------------------------------------------------------------
GameState* AppState::getNextState()
{
   switch (nextState)
   {
      case States::None:
         return nullptr;
         break;
      case States::MainMenu:
         return nullptr;
         break;
      case States::Drawing:
         return new AppState();
         break;
      case States::Quit:
         return new QuitState();
         break;
      default:
         return nullptr;
         break;
   }
}

// public ----------------------------------------------------------------------------------------------------
void AppState::currentCharacterButtonFunc()
{
   // Unhighlight previously selected button
   if (currentSelectedButton)
   {
      currentSelectedButton->unhighlight();
   }

   // Set current button as selected and highlight it
   currentSelectedButton = currentCharacterButton;
   currentCharacterButton->highlight(RGB(250, 250, 250));

   selectNewCharacter = true;
}

// public ----------------------------------------------------------------------------------------------------
void AppState::eraserSelectButtonFunc()
{
   // Unhighlight previously selected button
   if (currentSelectedButton)
   {
      currentSelectedButton->unhighlight();
   }

   // Set eraser button as selected and highlight it
   currentSelectedButton = eraserSelectButton;
   eraserSelectButton->highlight(RGB(250, 250, 250));

   drawingTool.setDrawingCharacter(' ');
   updateButtonStates();

   // Force display refresh to ensure proper erasing behavior
   displayNeedsCleared = true;
}

// public ----------------------------------------------------------------------------------------------------
void AppState::frameLengthButtonFunc() {}

// public ----------------------------------------------------------------------------------------------------
void AppState::brushSizeButtonFunc() {}

// public ----------------------------------------------------------------------------------------------------
void AppState::nextFrameButtonFunc()
{
   // Sync current frame to drawn entity before moving
   frameManager.syncFrameToDrawnEntity();

   if (visibleEntity->getCurrentAnimation().hasNextFrame())
   {
      // Get current frame to use as greyed background
      const Frame& currentFrame = visibleEntity->getCurrentAnimation().getFrameAtIndex(
            visibleEntity->getCurrentAnimation().getCurrentFrameIndex());

      // Move to next frame
      visibleEntity->getCurrentAnimationMutable().manuallyIncrementFrame();
      drawnEntity->getCurrentAnimationMutable().manuallyIncrementFrame();

      // Set the previous frame as greyed background
      frameManager.setGreyedBackground(currentFrame);
   }
   else
   {
      // Create new frame
      frameManager.createNewFrame();

      // Get current frame to use as greyed background
      const Frame& currentFrame = visibleEntity->getCurrentAnimation().getFrameAtIndex(
            visibleEntity->getCurrentAnimation().getCurrentFrameIndex());

      // Move to the new frame
      visibleEntity->getCurrentAnimationMutable().manuallyIncrementFrame();
      drawnEntity->getCurrentAnimationMutable().manuallyIncrementFrame();

      // Set the previous frame as greyed background
      frameManager.setGreyedBackground(currentFrame);
   }

   updateButtonStates();

   // Force display refresh
   displayNeedsCleared = true;

   // Update slider and button text for the new frame
   updateSliderFromFrameDuration();
   updateFrameDurationButtonText();
}

// public ----------------------------------------------------------------------------------------------------
void AppState::playAnimationButtonFunc()
{
   if (!visibleEntity->getCurrentAnimation().isPlaying())
   {
      // Start playing animation
      // Sync drawnEntity current frame to visibleEntity before playing
      frameManager.syncFrameToDrawnEntity();

      // Copy the complete animation from drawnEntity to visibleEntity for playback
      visibleEntity->getCurrentAnimationMutable() = drawnEntity->getCurrentAnimation();

      // Start the animation
      visibleEntity->getCurrentAnimationMutable().setPlaying(true);

      // Hide greyed background during playback
      frameManager.clearGreyedBackground();

      // Update button text
      playAnimationButton->setText("Stop Animation");

      // Update button states - disable editing controls during playback
      updateButtonStates();
   }
   else
   {
      // Stop playing animation
      visibleEntity->getCurrentAnimationMutable().setPlaying(false);

      // Sync the current frame back to drawnEntity after stopping
      // Get the current frame index from visibleEntity
      size_t currentIndex = visibleEntity->getCurrentAnimation().getCurrentFrameIndex();
      size_t drawnIndex   = drawnEntity->getCurrentAnimation().getCurrentFrameIndex();

      // Navigate drawnEntity to the same frame as visibleEntity
      while (drawnIndex != currentIndex)
      {
         if (drawnIndex < currentIndex)
         {
            drawnEntity->getCurrentAnimationMutable().manuallyIncrementFrame();
            drawnIndex++;
         }
         else
         {
            drawnEntity->getCurrentAnimationMutable().manuallyDecrementFrame();
            drawnIndex--;
         }
      }

      // Update button text
      playAnimationButton->setText("Play Animation");

      // Show greyed background of previous frame if not on first frame
      if (currentIndex > 0)
      {
         const Frame& frameBefore = visibleEntity->getCurrentAnimation().getFrameAtIndex(currentIndex - 1);
         frameManager.setGreyedBackground(frameBefore);
      }
      else
      {
         frameManager.clearGreyedBackground();
      }

      // Update button states - re-enable editing controls
      updateButtonStates();
   }
}

// public ----------------------------------------------------------------------------------------------------
void AppState::previousFrameButtonFunc()
{
   // Sync current frame to drawn entity before moving
   frameManager.syncFrameToDrawnEntity();

   if (visibleEntity->getCurrentAnimation().hasPreviousFrame())
   {
      // Move to previous frame
      visibleEntity->getCurrentAnimationMutable().manuallyDecrementFrame();
      drawnEntity->getCurrentAnimationMutable().manuallyDecrementFrame();

      // Get the frame before the current one to use as greyed background (if it exists)
      size_t currentIndex = visibleEntity->getCurrentAnimation().getCurrentFrameIndex();
      if (currentIndex > 0)
      {
         const Frame& frameBefore = visibleEntity->getCurrentAnimation().getFrameAtIndex(currentIndex - 1);

         // Set the frame before as greyed background
         frameManager.setGreyedBackground(frameBefore);
      }
      else
      {
         // No frame before, clear greyed background
         frameManager.clearGreyedBackground();
      }
   }

   updateButtonStates();

   // Force display refresh
   displayNeedsCleared = true;

   // Update slider and button text for the new frame
   updateSliderFromFrameDuration();
   updateFrameDurationButtonText();
}

// public ----------------------------------------------------------------------------------------------------
void AppState::quitButtonFunc()
{
   nextState = States::Quit;
}

// public ----------------------------------------------------------------------------------------------------
void AppState::backgroundColorButtonFunc()
{
   // Background color selection functionality to be implemented
}

// public ----------------------------------------------------------------------------------------------------

// public ----------------------------------------------------------------------------------------------------
void AppState::updateButtonStates()
{
   bool isPlaying = visibleEntity->getCurrentAnimation().isPlaying();

   // During playback, disable most editing controls
   if (isPlaying)
   {
      // Disable frame navigation buttons during playback
      nextFrameButton->setVisability(false);
      previousFrameButton->setVisability(false);

      // Disable drawing tool buttons during playback
      currentCharacterButton->setVisability(false);
      eraserSelectButton->setVisability(false);
      brushSizeButton->setVisability(false);

      // Disable frame duration editing during playback
      frameDurationSlider->setVisability(false);
      frameLengthButton->setVisability(false);

      // Disable brush size editing during playback
      brushSizeSlider->setVisability(false);

      // Keep other buttons available
      playAnimationButton->setVisability(true);
      currentColorsButton->setVisability(true);
      quitButton->setVisability(true);
   }
   else
   {
      // Normal editing mode - enable all controls

      // Update next frame button
      if (frameManager.frameHasContent())
      {
         nextFrameButton->setVisability(true);

         if (visibleEntity->getCurrentAnimation().hasNextFrame())
         {
            nextFrameButton->setText("Next Frame");
         }
         else
         {
            nextFrameButton->setText("Create New Next Frame");
         }
      }
      else
      {
         nextFrameButton->setVisability(false);
      }

      // Update previous frame button
      if (visibleEntity->getCurrentAnimation().hasPreviousFrame())
      {
         previousFrameButton->setVisability(true);
         previousFrameButton->setText("Previous Frame");
      }
      else
      {
         previousFrameButton->setVisability(false);
      }

      // Enable drawing tool buttons
      currentCharacterButton->setVisability(true);
      eraserSelectButton->setVisability(true);
      brushSizeButton->setVisability(true);

      // Enable frame duration editing
      frameDurationSlider->setVisability(true);
      frameLengthButton->setVisability(true);

      // Enable brush size editing
      brushSizeSlider->setVisability(true);

      // Enable all other buttons
      playAnimationButton->setVisability(true);
      currentColorsButton->setVisability(true);
      quitButton->setVisability(true);

      // Clear greyed background if we're on the first frame
      if (visibleEntity->getCurrentAnimation().getCurrentFrameIndex() == 0)
      {
         frameManager.clearGreyedBackground();
      }
   }

   // Refresh all locked positions after text changes
   UIElement::updateAllLockedPositions();
}

// public ----------------------------------------------------------------------------------------------------
// public ----------------------------------------------------------------------------------------------------
void AppState::updateButtonHighlighting(const Position& mousePos, bool isMousePressed)
{
   std::shared_ptr<Button> buttonAtPosition = getButtonAtPosition(mousePos);

   // Handle hover highlighting
   if (buttonAtPosition != currentHoveredButton)
   {
      // Unhighlight previously hovered button (if not clicked and not selected)
      if (currentHoveredButton && currentHoveredButton != currentClickedButton &&
          currentHoveredButton != currentSelectedButton)
      {
         currentHoveredButton->unhighlight();
      }

      // Highlight new button (if not selected)
      currentHoveredButton = buttonAtPosition;
      if (currentHoveredButton && currentHoveredButton != currentClickedButton &&
          currentHoveredButton != currentSelectedButton)
      {
         currentHoveredButton->highlight(RGB(750, 750, 750));
      }
   }

   // Handle click highlighting
   if (isMousePressed && buttonAtPosition)
   {
      // Unhighlight previously clicked button (if not selected)
      if (currentClickedButton && currentClickedButton != buttonAtPosition &&
          currentClickedButton != currentSelectedButton)
      {
         currentClickedButton->unhighlight();
      }

      // Highlight clicked button (if not selected)
      currentClickedButton = buttonAtPosition;
      if (currentClickedButton != currentSelectedButton)
      {
         currentClickedButton->highlight(RGB(500, 500, 500));
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
std::shared_ptr<Button> AppState::getButtonAtPosition(const Position& position)
{
   std::vector<std::shared_ptr<Button>> allButtons = {currentColorsButton, currentCharacterButton,
                                                      eraserSelectButton,  brushSizeButton,
                                                      frameLengthButton,   nextFrameButton,
                                                      playAnimationButton, previousFrameButton,
                                                      quitButton};

   for (auto& button : allButtons)
   {
      if (button && button->isVisable() && button->mouseInBounds(position))
      {
         return button;
      }
   }

   return nullptr;
}

// public ----------------------------------------------------------------------------------------------------
void AppState::updateFrameDurationFromSlider()
{
   float sliderValue = frameDurationSlider->getValue();
   float newDuration = sliderValueToDuration(sliderValue);

   // Update current frame duration
   size_t currentIndex = visibleEntity->getCurrentAnimation().getCurrentFrameIndex();

   // Create new frames with updated duration
   Frame& currentVisibleFrame =
         visibleEntity->getCurrentAnimationMutable().getFrameAtIndexMutable(currentIndex);
   Frame& currentDrawnFrame = drawnEntity->getCurrentAnimationMutable().getFrameAtIndexMutable(currentIndex);

   Frame newVisibleFrame(currentVisibleFrame.getSprite(), newDuration);
   Frame newDrawnFrame(currentDrawnFrame.getSprite(), newDuration);

   // Replace the frames
   visibleEntity->getCurrentAnimationMutable().getFrameAtIndexMutable(currentIndex) = newVisibleFrame;
   drawnEntity->getCurrentAnimationMutable().getFrameAtIndexMutable(currentIndex)   = newDrawnFrame;
}

// public ----------------------------------------------------------------------------------------------------
void AppState::updateSliderFromFrameDuration()
{
   size_t currentIndex    = visibleEntity->getCurrentAnimation().getCurrentFrameIndex();
   float  currentDuration = visibleEntity->getCurrentAnimation().getFrameAtIndex(currentIndex).getDuration();
   float  sliderValue     = durationToSliderValue(currentDuration);

   // Convert slider value to position
   int sliderPosition = static_cast<int>(sliderValue * (frameDurationSlider->getLength() - 1));
   frameDurationSlider->setPosition(sliderPosition);
}

// public ----------------------------------------------------------------------------------------------------
void AppState::updateFrameDurationButtonText()
{
   size_t currentIndex    = visibleEntity->getCurrentAnimation().getCurrentFrameIndex();
   float  currentDuration = visibleEntity->getCurrentAnimation().getFrameAtIndex(currentIndex).getDuration();

   // Format to 2 decimal places
   std::string durationText = std::to_string(currentDuration);
   size_t      dotPos       = durationText.find('.');
   if (dotPos != std::string::npos && dotPos + 3 < durationText.length())
   {
      durationText = durationText.substr(0, dotPos + 3);
   }

   std::string buttonText = "Duration: " + durationText + "s";
   frameLengthButton->setText(buttonText);
   UIElement::updateAllLockedPositions();
}

// public ----------------------------------------------------------------------------------------------------
float AppState::durationToSliderValue(float duration) const
{
   // Clamp duration to valid range
   duration = std::max(0.1f, std::min(5.0f, duration));

   // Map from [0.1, 5.0] to [0.0, 1.0]
   return (duration - 0.1f) / 4.9f;
}

// public ----------------------------------------------------------------------------------------------------
float AppState::sliderValueToDuration(float sliderValue) const
{
   // Clamp slider value to valid range
   sliderValue = std::max(0.0f, std::min(1.0f, sliderValue));

   // Map from [0.0, 1.0] to [0.1, 5.0]
   return 0.1f + (sliderValue * 4.9f);
}

// public ----------------------------------------------------------------------------------------------------
void AppState::updateBrushSizeFromSlider()
{
   float sliderValue = brushSizeSlider->getValue();

   // Convert slider value (0.0-1.0) to brush size (1-6)
   int brushSize = static_cast<int>(sliderValue * 5.0f) + 1;

   // Clamp to valid range
   brushSize = std::max(1, std::min(6, brushSize));
   drawingTool.setBrushSize(brushSize);
}

// public ----------------------------------------------------------------------------------------------------
// public ----------------------------------------------------------------------------------------------------
void AppState::updateBrushSizeButtonText()
{
   std::string buttonText = "Brush size: " + std::to_string(drawingTool.getBrushSize()) + " characters";
   brushSizeButton->setText(buttonText);
   UIElement::updateAllLockedPositions();
}

// public ----------------------------------------------------------------------------------------------------
// public ----------------------------------------------------------------------------------------------------
void AppState::currentColorsButtonFunc()
{
   if (!colorEditWindowOpen)
   {
      createColorEditWindow();
   }
}

// public ----------------------------------------------------------------------------------------------------
void AppState::colorExitButtonFunc()
{
   closeColorEditWindow();
}

// public ----------------------------------------------------------------------------------------------------
void AppState::backgroundColorDisplayButtonFunc()
{
   // Empty function - button is just for display
}

// public ----------------------------------------------------------------------------------------------------
void AppState::textColorDisplayButtonFunc()
{
   // Empty function - button is just for display
}

// public ----------------------------------------------------------------------------------------------------
void AppState::exampleTextButtonFunc()
{
   // Empty function - button is just for display
}

// public ----------------------------------------------------------------------------------------------------
void AppState::backgroundRGBButtonFunc()
{
   // Empty function - button is just for display
}

// public ----------------------------------------------------------------------------------------------------
void AppState::textRGBButtonFunc()
{
   // Empty function - button is just for display
}

// public ----------------------------------------------------------------------------------------------------
void AppState::createColorEditWindow()
{
   if (colorEditWindowOpen)
   {
      return;
   }

   // Create the color edit window
   colorEditWindow = std::make_shared<NcursesWindow>(80, 30, 2, false, 5, 5);
   colorEditWindow->setBorderEnabled(true);
   colorEditWindowOpen = true;

   // Add window to display system
   ncursesWindows.push_back(colorEditWindow);

   // Create background color sliders (left side)
   backgroundRedSlider   = std::make_shared<Slider>(21, true); // 21 positions for ~50 increments per tick
   backgroundGreenSlider = std::make_shared<Slider>(21, true);
   backgroundBlueSlider  = std::make_shared<Slider>(21, true);

   // Create text color sliders (right side)
   textRedSlider   = std::make_shared<Slider>(21, true);
   textGreenSlider = std::make_shared<Slider>(21, true);
   textBlueSlider  = std::make_shared<Slider>(21, true);

   // Create buttons
   backgroundColorDisplayButton =
         PrintableFactory::newButton("Background Color:\n       ",
                                     &AppState::backgroundColorDisplayButtonFunc, this, colorEditWindow);
   textColorDisplayButton =
         PrintableFactory::newButton("Text Color:\n       ", &AppState::textColorDisplayButtonFunc, this,
                                     colorEditWindow);
   exampleTextButton   = PrintableFactory::newButton("Example text:\nThis is an example.",
                                                     &AppState::exampleTextButtonFunc, this, colorEditWindow);
   backgroundRGBButton = PrintableFactory::newButton("R=0, G=0, B=0", &AppState::backgroundRGBButtonFunc,
                                                     this, colorEditWindow);
   textRGBButton = PrintableFactory::newButton("R=100, G=100, B=100", &AppState::textRGBButtonFunc, this,
                                               colorEditWindow);
   colorExitButton = PrintableFactory::newButton("Exit", &AppState::colorExitButtonFunc, this, colorEditWindow);

   // Set positions
   backgroundRedSlider->setDynamicPosition(ScreenLockPosition::BOTTOM_LEFT_CORNER, StackDirection::VERTICAL);
   backgroundGreenSlider->setDynamicPosition(ScreenLockPosition::BOTTOM_LEFT_CORNER, StackDirection::VERTICAL);
   backgroundBlueSlider->setDynamicPosition(ScreenLockPosition::BOTTOM_LEFT_CORNER, StackDirection::VERTICAL);

   textRedSlider->setDynamicPosition(ScreenLockPosition::BOTTOM_RIGHT_CORNER, StackDirection::VERTICAL);
   textGreenSlider->setDynamicPosition(ScreenLockPosition::BOTTOM_RIGHT_CORNER, StackDirection::VERTICAL);
   textBlueSlider->setDynamicPosition(ScreenLockPosition::BOTTOM_RIGHT_CORNER, StackDirection::VERTICAL);

   backgroundColorDisplayButton->setDynamicPosition(ScreenLockPosition::CENTER, StackDirection::VERTICAL);
   textColorDisplayButton->setDynamicPosition(ScreenLockPosition::CENTER, StackDirection::VERTICAL);
   exampleTextButton->setDynamicPosition(ScreenLockPosition::CENTER, StackDirection::VERTICAL);

   backgroundRGBButton->setDynamicPosition(ScreenLockPosition::BOTTOM_LEFT_CORNER, StackDirection::VERTICAL);
   textRGBButton->setDynamicPosition(ScreenLockPosition::BOTTOM_RIGHT_CORNER, StackDirection::VERTICAL);
   colorExitButton->setDynamicPosition(ScreenLockPosition::TOP_LEFT_CORNER);

   // Set visibility
   backgroundRedSlider->setVisability(true);
   backgroundGreenSlider->setVisability(true);
   backgroundBlueSlider->setVisability(true);
   textRedSlider->setVisability(true);
   textGreenSlider->setVisability(true);
   textBlueSlider->setVisability(true);
   backgroundColorDisplayButton->setVisability(true);
   textColorDisplayButton->setVisability(true);
   exampleTextButton->setVisability(true);
   backgroundRGBButton->setVisability(true);
   textRGBButton->setVisability(true);
   colorExitButton->setVisability(true);

   // Set camera movement
   backgroundRedSlider->setMoveableByCamera(false);
   backgroundGreenSlider->setMoveableByCamera(false);
   backgroundBlueSlider->setMoveableByCamera(false);
   textRedSlider->setMoveableByCamera(false);
   textGreenSlider->setMoveableByCamera(false);
   textBlueSlider->setMoveableByCamera(false);
   backgroundColorDisplayButton->setMoveableByCamera(false);
   textColorDisplayButton->setMoveableByCamera(false);
   exampleTextButton->setMoveableByCamera(false);
   backgroundRGBButton->setMoveableByCamera(false);
   textRGBButton->setMoveableByCamera(false);
   colorExitButton->setMoveableByCamera(false);

   // Set layers
   backgroundRedSlider->setAllAnimationSpriteLayers(2);
   backgroundGreenSlider->setAllAnimationSpriteLayers(2);
   backgroundBlueSlider->setAllAnimationSpriteLayers(2);
   textRedSlider->setAllAnimationSpriteLayers(2);
   textGreenSlider->setAllAnimationSpriteLayers(2);
   textBlueSlider->setAllAnimationSpriteLayers(2);
   backgroundColorDisplayButton->setAllAnimationSpriteLayers(2);
   textColorDisplayButton->setAllAnimationSpriteLayers(2);
   exampleTextButton->setAllAnimationSpriteLayers(2);
   backgroundRGBButton->setAllAnimationSpriteLayers(2);
   textRGBButton->setAllAnimationSpriteLayers(2);
   colorExitButton->setAllAnimationSpriteLayers(2);

   // Set window for sliders and add to window (buttons are automatically added by PrintableFactory::newButton)
   backgroundRedSlider->setNcurseWindow(colorEditWindow->getWindow());
   backgroundGreenSlider->setNcurseWindow(colorEditWindow->getWindow());
   backgroundBlueSlider->setNcurseWindow(colorEditWindow->getWindow());
   textRedSlider->setNcurseWindow(colorEditWindow->getWindow());
   textGreenSlider->setNcurseWindow(colorEditWindow->getWindow());
   textBlueSlider->setNcurseWindow(colorEditWindow->getWindow());

   colorEditWindow->addPrintable(backgroundRedSlider);
   colorEditWindow->addPrintable(backgroundGreenSlider);
   colorEditWindow->addPrintable(backgroundBlueSlider);
   colorEditWindow->addPrintable(textRedSlider);
   colorEditWindow->addPrintable(textGreenSlider);
   colorEditWindow->addPrintable(textBlueSlider);

   // Register sliders with global input handler
   globalInputHandler.addSlider(backgroundRedSlider);
   globalInputHandler.addSlider(backgroundGreenSlider);
   globalInputHandler.addSlider(backgroundBlueSlider);
   globalInputHandler.addSlider(textRedSlider);
   globalInputHandler.addSlider(textGreenSlider);
   globalInputHandler.addSlider(textBlueSlider);

   // Initialize sliders with current colors
   updateSlidersFromColors();
   updateColorDisplayButtons();

   // Update positions
   UIElement::updateAllLockedPositions();
}

// public ----------------------------------------------------------------------------------------------------
void AppState::closeColorEditWindow()
{
   if (!colorEditWindowOpen)
   {
      return;
   }

   // Remove window from display system
   if (colorEditWindow)
   {
      Display::removeWindow(colorEditWindow);
      colorEditWindow->clearPrintables();

      // Remove the window from the ncursesWindows vector
      auto it = std::find(ncursesWindows.begin(), ncursesWindows.end(), colorEditWindow);
      if (it != ncursesWindows.end())
      {
         ncursesWindows.erase(it);
      }

      displayNeedsCleared = true;
   }

   colorEditWindowOpen = false;
   colorEditWindow     = nullptr;

   // Remove buttons and sliders from global input handler before clearing pointers
   if (backgroundRedSlider)
      globalInputHandler.removeSlider(backgroundRedSlider);
   if (backgroundGreenSlider)
      globalInputHandler.removeSlider(backgroundGreenSlider);
   if (backgroundBlueSlider)
      globalInputHandler.removeSlider(backgroundBlueSlider);
   if (textRedSlider)
      globalInputHandler.removeSlider(textRedSlider);
   if (textGreenSlider)
      globalInputHandler.removeSlider(textGreenSlider);
   if (textBlueSlider)
      globalInputHandler.removeSlider(textBlueSlider);
   if (backgroundColorDisplayButton)
      globalInputHandler.removeButton(backgroundColorDisplayButton);
   if (textColorDisplayButton)
      globalInputHandler.removeButton(textColorDisplayButton);
   if (exampleTextButton)
      globalInputHandler.removeButton(exampleTextButton);
   if (backgroundRGBButton)
      globalInputHandler.removeButton(backgroundRGBButton);
   if (textRGBButton)
      globalInputHandler.removeButton(textRGBButton);
   if (colorExitButton)
      globalInputHandler.removeButton(colorExitButton);

   // Clear pointers
   backgroundRedSlider          = nullptr;
   backgroundGreenSlider        = nullptr;
   backgroundBlueSlider         = nullptr;
   textRedSlider                = nullptr;
   textGreenSlider              = nullptr;
   textBlueSlider               = nullptr;
   backgroundColorDisplayButton = nullptr;
   textColorDisplayButton       = nullptr;
   exampleTextButton            = nullptr;
   backgroundRGBButton          = nullptr;
   textRGBButton                = nullptr;
   colorExitButton              = nullptr;
}

// public ----------------------------------------------------------------------------------------------------
void AppState::updateColorsFromSliders()
{
   if (!colorEditWindowOpen)
   {
      return;
   }

   // Update background color
   int backgroundRed      = sliderValueToRgb(backgroundRedSlider->getValue());
   int backgroundGreen    = sliderValueToRgb(backgroundGreenSlider->getValue());
   int backgroundBlue     = sliderValueToRgb(backgroundBlueSlider->getValue());
   currentBackgroundColor = RGB(backgroundRed, backgroundGreen, backgroundBlue);

   // Update text color
   int textRed      = sliderValueToRgb(textRedSlider->getValue());
   int textGreen    = sliderValueToRgb(textGreenSlider->getValue());
   int textBlue     = sliderValueToRgb(textBlueSlider->getValue());
   currentTextColor = RGB(textRed, textGreen, textBlue);

   // Update the drawing tool with the new colors
   drawingTool.setTextColor(currentTextColor);
   drawingTool.setBackgroundColor(currentBackgroundColor);

   // Update display buttons
   updateColorDisplayButtons();
   updateCurrentColorsButtonText();
}

// public ----------------------------------------------------------------------------------------------------
void AppState::updateSlidersFromColors()
{
   if (!colorEditWindowOpen)
   {
      return;
   }

   // Update background sliders
   float backgroundRedValue   = rgbToSliderValue(currentBackgroundColor.getR());
   float backgroundGreenValue = rgbToSliderValue(currentBackgroundColor.getG());
   float backgroundBlueValue  = rgbToSliderValue(currentBackgroundColor.getB());

   int backgroundRedPosition = static_cast<int>(backgroundRedValue * (backgroundRedSlider->getLength() - 1));
   int backgroundGreenPosition =
         static_cast<int>(backgroundGreenValue * (backgroundGreenSlider->getLength() - 1));
   int backgroundBluePosition = static_cast<int>(backgroundBlueValue * (backgroundBlueSlider->getLength() - 1));

   backgroundRedSlider->setPosition(backgroundRedPosition);
   backgroundGreenSlider->setPosition(backgroundGreenPosition);
   backgroundBlueSlider->setPosition(backgroundBluePosition);

   // Update text sliders
   float textRedValue   = rgbToSliderValue(currentTextColor.getR());
   float textGreenValue = rgbToSliderValue(currentTextColor.getG());
   float textBlueValue  = rgbToSliderValue(currentTextColor.getB());

   int textRedPosition   = static_cast<int>(textRedValue * (textRedSlider->getLength() - 1));
   int textGreenPosition = static_cast<int>(textGreenValue * (textGreenSlider->getLength() - 1));
   int textBluePosition  = static_cast<int>(textBlueValue * (textBlueSlider->getLength() - 1));

   textRedSlider->setPosition(textRedPosition);
   textGreenSlider->setPosition(textGreenPosition);
   textBlueSlider->setPosition(textBluePosition);
}

// public ----------------------------------------------------------------------------------------------------
void AppState::updateColorDisplayButtons()
{
   if (!colorEditWindowOpen)
   {
      return;
   }

   // Update background color display button
   std::string backgroundText = "Background Color:\n       ";
   backgroundColorDisplayButton->setText(backgroundText);
   // Only the space characters on the second line get the background color
   setButtonBackgroundColorDisplay(backgroundColorDisplayButton, currentBackgroundColor);

   // Update text color display button
   std::string textText = "Text Color:\n       ";
   textColorDisplayButton->setText(textText);
   // Only the space characters on the second line get the text color
   setButtonTextColorDisplay(textColorDisplayButton, currentTextColor);

   // Update example text button
   std::string exampleText = "Example text:\nThis is an example.";
   exampleTextButton->setText(exampleText);
   // Only the "This is an example." text gets both text and background colors
   setButtonExampleTextColors(exampleTextButton, currentTextColor, currentBackgroundColor);

   // Update RGB value buttons
   int         backgroundRed     = currentBackgroundColor.getR() / 10; // Convert from 0-1000 to 0-100
   int         backgroundGreen   = currentBackgroundColor.getG() / 10;
   int         backgroundBlue    = currentBackgroundColor.getB() / 10;
   std::string backgroundRGBText = "R=" + std::to_string(backgroundRed) +
                                   ", G=" + std::to_string(backgroundGreen) +
                                   ", B=" + std::to_string(backgroundBlue);
   backgroundRGBButton->setText(backgroundRGBText);

   int         textRed     = currentTextColor.getR() / 10;
   int         textGreen   = currentTextColor.getG() / 10;
   int         textBlue    = currentTextColor.getB() / 10;
   std::string textRGBText = "R=" + std::to_string(textRed) + ", G=" + std::to_string(textGreen) +
                             ", B=" + std::to_string(textBlue);
   textRGBButton->setText(textRGBText);

   UIElement::updateAllLockedPositions();
}

// public ----------------------------------------------------------------------------------------------------
void AppState::updateCurrentColorsButtonText()
{
   std::string buttonText =
         "Current Colors: " + std::string(1, drawingTool.getDrawingCharacter()) + "\nClick to Edit";
   currentColorsButton->setText(buttonText);
   // Only apply colors to the drawing character
   setCurrentColorsButtonColors(currentColorsButton, currentTextColor, currentBackgroundColor,
                                drawingTool.getDrawingCharacter());
   UIElement::updateAllLockedPositions();
}

// public ----------------------------------------------------------------------------------------------------
float AppState::rgbToSliderValue(int rgbValue) const
{
   // Convert from 0-1000 to 0-100 to 0.0-1.0
   float percentage = static_cast<float>(rgbValue) / 1000.0f;
   return percentage;
}

// public ----------------------------------------------------------------------------------------------------
int AppState::sliderValueToRgb(float sliderValue) const
{
   // Convert from 0.0-1.0 to 0-1000
   int rgbValue = static_cast<int>(sliderValue * 1000.0f);
   return std::max(0, std::min(1000, rgbValue));
}

// public ----------------------------------------------------------------------------------------------------
void AppState::setButtonBackgroundColorDisplay(std::shared_ptr<Button> button, const RGB& backgroundColor)
{
   if (!button)
   {
      return;
   }

   // Get the button's current sprite
   Sprite&             sprite = button->getCurrentAnimationMutable().getCurrentFrameSpriteMutable();
   std::vector<Pixel>& pixels = sprite.getPixelsMutable();

   // For "Background Color:\n       " we want to color the spaces after the newline
   // Find the position of any colon character to determine where the first line ends
   int colonY = -1;
   for (const Pixel& pixel : pixels)
   {
      if (pixel.getCharacter() == ':')
      {
         colonY = pixel.getPosition().getY();
         break;
      }
   }

   // Color space characters that are on the line after the colon
   for (Pixel& pixel : pixels)
   {
      if (pixel.getCharacter() == ' ')
      {
         // Check if this space is on the second line (Y position greater than colon's Y)
         if (colonY >= 0 && pixel.getPosition().getY() > colonY)
         {
            // Create a new pixel with the same properties but with the new background color
            Pixel newPixel(pixel.getPosition(), pixel.getCharacter(), pixel.getTextColor(), backgroundColor,
                           pixel.getAttributes());
            pixel = newPixel;
         }
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
void AppState::setButtonTextColorDisplay(std::shared_ptr<Button> button, const RGB& textColor)
{
   if (!button)
   {
      return;
   }

   // Get the button's current sprite
   Sprite&             sprite = button->getCurrentAnimationMutable().getCurrentFrameSpriteMutable();
   std::vector<Pixel>& pixels = sprite.getPixelsMutable();

   // For "Text Color:\n       " we want to color the spaces after the newline
   // Find the position of any colon character to determine where the first line ends
   int colonY = -1;
   for (const Pixel& pixel : pixels)
   {
      if (pixel.getCharacter() == ':')
      {
         colonY = pixel.getPosition().getY();
         break;
      }
   }

   // Color space characters that are on the line after the colon
   for (Pixel& pixel : pixels)
   {
      if (pixel.getCharacter() == ' ')
      {
         // Check if this space is on the second line (Y position greater than colon's Y)
         if (colonY >= 0 && pixel.getPosition().getY() > colonY)
         {
            // Create a new pixel with the same text color but with textColor as background
            Pixel newPixel(pixel.getPosition(), pixel.getCharacter(), pixel.getTextColor(), textColor,
                           pixel.getAttributes());
            pixel = newPixel;
         }
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
void AppState::setButtonExampleTextColors(std::shared_ptr<Button> button, const RGB& textColor,
                                          const RGB& backgroundColor)
{
   if (!button)
   {
      return;
   }

   // Get the button's current sprite
   Sprite&             sprite = button->getCurrentAnimationMutable().getCurrentFrameSpriteMutable();
   std::vector<Pixel>& pixels = sprite.getPixelsMutable();

   // For "Example text:\nThis is an example." we want to color "This is an example." after the newline
   // Find the position of any colon character to determine where the first line ends
   int colonY = -1;
   for (const Pixel& pixel : pixels)
   {
      if (pixel.getCharacter() == ':')
      {
         colonY = pixel.getPosition().getY();
         break;
      }
   }

   // Color all characters that are on the line after the colon (second line)
   for (Pixel& pixel : pixels)
   {
      // Check if this character is on the second line (Y position greater than colon's Y)
      if (colonY >= 0 && pixel.getPosition().getY() > colonY)
      {
         // Apply colors to all characters on the second line
         Pixel newPixel(pixel.getPosition(), pixel.getCharacter(), textColor, backgroundColor,
                        pixel.getAttributes());
         pixel = newPixel;
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
void AppState::setCurrentColorsButtonColors(std::shared_ptr<Button> button, const RGB& textColor,
                                            const RGB& backgroundColor, char drawingChar)
{
   if (!button)
   {
      return;
   }

   // Get the button's current sprite
   Sprite&             sprite = button->getCurrentAnimationMutable().getCurrentFrameSpriteMutable();
   std::vector<Pixel>& pixels = sprite.getPixelsMutable();

   // Find only the drawing character and set its colors
   for (Pixel& pixel : pixels)
   {
      // Check if this pixel is the drawing character
      if (pixel.getCharacter() == drawingChar)
      {
         // Create a new pixel with the updated colors
         Pixel newPixel(pixel.getPosition(), pixel.getCharacter(), textColor, backgroundColor,
                        pixel.getAttributes());
         pixel = newPixel;
      }
   }
}