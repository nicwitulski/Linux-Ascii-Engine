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
   brushSizeButton    = PrintableFactory::newButton("Brush size: 1 characters");
   frameLengthButton  = PrintableFactory::newButton("Frame Length");
   nextFrameButton    = PrintableFactory::newButton("Next Frame", &AppState::nextFrameButtonFunc, this);
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

   // Use automatic highlighting system - no manual state tracking needed

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
   globalInputHandler.setSelectedButton(currentCharacterButton);

   // Update slider and button text based on current frame duration
   updateSliderFromFrameDuration();
   updateFrameDurationButtonText();

   // Update brush size slider and button text
   updateBrushSizeButtonText();

   // Update colors button text
   updateCurrentColorsButtonText();

   // Remove borders from label buttons AFTER all text updates
   brushSizeButton->setBorder(false);
   frameLengthButton->setBorder(false);
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

         // Keep the currentCharacterButton selected to show it's active
         globalInputHandler.setSelectedButton(currentCharacterButton);

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

         // Button highlighting is now handled automatically by InputHandler
      }
      else if (userInput == 27) // Escape key
      {
         // Cancel editing
         editingFrameDuration = false;
         frameDurationInput   = "";

         // Button highlighting is now handled automatically by InputHandler
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

         // Button highlighting is now handled automatically by the InputHandler

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
            // Button highlighting is now handled automatically by the InputHandler
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
            // Button highlighting is now handled automatically by the InputHandler

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

   // Update the current colors button to show the selected colors every frame
   // This ensures the custom colors are maintained even after button highlighting
   setCurrentColorsButtonColors(currentColorsButton, currentTextColor, currentBackgroundColor,
                                drawingTool.getDrawingCharacter());
}

// public ----------------------------------------------------------------------------------------------------
void AppState::onExit()
{
   clear();
   ncursesWindows.at(0)->clearPrintables();
   globalInputHandler.clear();

   // Button highlighting cleanup is now handled automatically by the InputHandler

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
   // Set current button as selected using automatic highlighting
   globalInputHandler.setSelectedButton(currentCharacterButton);

   // Stop any ongoing drawing operation when entering character selection mode
   mouseHandler.stopDrawing();

   selectNewCharacter = true;
}

// public ----------------------------------------------------------------------------------------------------
void AppState::eraserSelectButtonFunc()
{
   // Set eraser button as selected using automatic highlighting
   globalInputHandler.setSelectedButton(eraserSelectButton);

   drawingTool.setDrawingCharacter(' ');
   updateButtonStates();

   // Force display refresh to ensure proper erasing behavior
   displayNeedsCleared = true;
}

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

      // Force display refresh to ensure greyed background appears when stopping animation
      displayNeedsCleared = true;

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
// Manual button highlighting methods removed - now handled automatically by InputHandler

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
   frameLengthButton->setBorder(false); // Maintain borderless state
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
   brushSizeButton->setBorder(false); // Maintain borderless state
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
void AppState::createColorEditWindow()
{
   if (colorEditWindowOpen)
   {
      return;
   }

   // Create the color edit window (smaller size)
   colorEditWindow = std::make_shared<NcursesWindow>(50, 20, 2, false, 10, 5);
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

   // Create simple labels for sliders
   backgroundRedLabel = PrintableFactory::newButton("Background Red", colorEditWindow);
   backgroundRedLabel->setBorder(false);
   backgroundGreenLabel = PrintableFactory::newButton("Background Green", colorEditWindow);
   backgroundGreenLabel->setBorder(false);
   backgroundBlueLabel = PrintableFactory::newButton("Background Blue", colorEditWindow);
   backgroundBlueLabel->setBorder(false);
   textRedLabel = PrintableFactory::newButton("Text Red", colorEditWindow);
   textRedLabel->setBorder(false);
   textGreenLabel = PrintableFactory::newButton("Text Green", colorEditWindow);
   textGreenLabel->setBorder(false);
   textBlueLabel = PrintableFactory::newButton("Text Blue", colorEditWindow);
   textBlueLabel->setBorder(false);
   colorExitButton = PrintableFactory::newButton("Exit", &AppState::colorExitButtonFunc, this, colorEditWindow);

   // Label buttons automatically have highlighting disabled and borders hidden since they have no function

   // Set positions - organized in two columns
   backgroundRedLabel->setDynamicPosition(ScreenLockPosition::LEFT_MIDDLE, StackDirection::VERTICAL);
   backgroundRedSlider->setDynamicPosition(ScreenLockPosition::LEFT_MIDDLE, StackDirection::VERTICAL);
   backgroundGreenLabel->setDynamicPosition(ScreenLockPosition::LEFT_MIDDLE, StackDirection::VERTICAL);
   backgroundGreenSlider->setDynamicPosition(ScreenLockPosition::LEFT_MIDDLE, StackDirection::VERTICAL);
   backgroundBlueLabel->setDynamicPosition(ScreenLockPosition::LEFT_MIDDLE, StackDirection::VERTICAL);
   backgroundBlueSlider->setDynamicPosition(ScreenLockPosition::LEFT_MIDDLE, StackDirection::VERTICAL);

   textRedLabel->setDynamicPosition(ScreenLockPosition::RIGHT_MIDDLE, StackDirection::VERTICAL);
   textRedSlider->setDynamicPosition(ScreenLockPosition::RIGHT_MIDDLE, StackDirection::VERTICAL);
   textGreenLabel->setDynamicPosition(ScreenLockPosition::RIGHT_MIDDLE, StackDirection::VERTICAL);
   textGreenSlider->setDynamicPosition(ScreenLockPosition::RIGHT_MIDDLE, StackDirection::VERTICAL);
   textBlueLabel->setDynamicPosition(ScreenLockPosition::RIGHT_MIDDLE, StackDirection::VERTICAL);
   textBlueSlider->setDynamicPosition(ScreenLockPosition::RIGHT_MIDDLE, StackDirection::VERTICAL);

   colorExitButton->setDynamicPosition(ScreenLockPosition::TOP_LEFT_CORNER);

   // Set visibility
   backgroundRedSlider->setVisability(true);
   backgroundGreenSlider->setVisability(true);
   backgroundBlueSlider->setVisability(true);
   textRedSlider->setVisability(true);
   textGreenSlider->setVisability(true);
   textBlueSlider->setVisability(true);
   backgroundRedLabel->setVisability(true);
   backgroundGreenLabel->setVisability(true);
   backgroundBlueLabel->setVisability(true);
   textRedLabel->setVisability(true);
   textGreenLabel->setVisability(true);
   textBlueLabel->setVisability(true);
   colorExitButton->setVisability(true);

   // Set camera movement
   backgroundRedSlider->setMoveableByCamera(false);
   backgroundGreenSlider->setMoveableByCamera(false);
   backgroundBlueSlider->setMoveableByCamera(false);
   textRedSlider->setMoveableByCamera(false);
   textGreenSlider->setMoveableByCamera(false);
   textBlueSlider->setMoveableByCamera(false);
   backgroundRedLabel->setMoveableByCamera(false);
   backgroundGreenLabel->setMoveableByCamera(false);
   backgroundBlueLabel->setMoveableByCamera(false);
   textRedLabel->setMoveableByCamera(false);
   textGreenLabel->setMoveableByCamera(false);
   textBlueLabel->setMoveableByCamera(false);
   colorExitButton->setMoveableByCamera(false);

   // Set layers
   backgroundRedSlider->setAllAnimationSpriteLayers(2);
   backgroundGreenSlider->setAllAnimationSpriteLayers(2);
   backgroundBlueSlider->setAllAnimationSpriteLayers(2);
   textRedSlider->setAllAnimationSpriteLayers(2);
   textGreenSlider->setAllAnimationSpriteLayers(2);
   textBlueSlider->setAllAnimationSpriteLayers(2);
   backgroundRedLabel->setAllAnimationSpriteLayers(2);
   backgroundGreenLabel->setAllAnimationSpriteLayers(2);
   backgroundBlueLabel->setAllAnimationSpriteLayers(2);
   textRedLabel->setAllAnimationSpriteLayers(2);
   textGreenLabel->setAllAnimationSpriteLayers(2);
   textBlueLabel->setAllAnimationSpriteLayers(2);
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
   if (backgroundRedLabel)
      globalInputHandler.removeButton(backgroundRedLabel);
   if (backgroundGreenLabel)
      globalInputHandler.removeButton(backgroundGreenLabel);
   if (backgroundBlueLabel)
      globalInputHandler.removeButton(backgroundBlueLabel);
   if (textRedLabel)
      globalInputHandler.removeButton(textRedLabel);
   if (textGreenLabel)
      globalInputHandler.removeButton(textGreenLabel);
   if (textBlueLabel)
      globalInputHandler.removeButton(textBlueLabel);
   if (colorExitButton)
      globalInputHandler.removeButton(colorExitButton);

   // Clear pointers
   backgroundRedSlider   = nullptr;
   backgroundGreenSlider = nullptr;
   backgroundBlueSlider  = nullptr;
   textRedSlider         = nullptr;
   textGreenSlider       = nullptr;
   textBlueSlider        = nullptr;
   backgroundRedLabel    = nullptr;
   backgroundGreenLabel  = nullptr;
   backgroundBlueLabel   = nullptr;
   textRedLabel          = nullptr;
   textGreenLabel        = nullptr;
   textBlueLabel         = nullptr;
   colorExitButton       = nullptr;
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

   // Update the current colors button
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
void AppState::setCurrentColorsButtonColors(std::shared_ptr<Button> button, const RGB& textColor,
                                            const RGB& backgroundColor, char /* drawingChar */)
{
   if (!button)
   {
      return;
   }

   // Get the button's current sprite
   Sprite&             sprite = button->getCurrentAnimationMutable().getCurrentFrameSpriteMutable();
   std::vector<Pixel>& pixels = sprite.getPixelsMutable();

   // Find the colon character in "Current Colors: x", then find the character 2 positions to the right
   // This should be the example character after ": "
   Position colonPos(-1, -1);

   // First pass: find the colon character
   for (const Pixel& pixel : pixels)
   {
      if (pixel.getCharacter() == ':')
      {
         colonPos = pixel.getPosition();
         break;
      }
   }

   // Second pass: find the character 2 positions to the right of the colon (after ": ")
   if (colonPos.getX() >= 0)
   {
      for (Pixel& pixel : pixels)
      {
         if (pixel.getPosition().getX() == colonPos.getX() + 2 && pixel.getPosition().getY() == colonPos.getY())
         {
            // Create a new pixel with the updated colors
            Pixel newPixel(pixel.getPosition(), pixel.getCharacter(), textColor, backgroundColor,
                           pixel.getAttributes());
            pixel = newPixel;
            break; // Only update the one example character
         }
      }
   }
}