//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file PrintableFactory.h
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Factory class for loading and creating printable objects from files
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRINTABLEFACTORY_H
#define PRINTABLEFACTORY_H

#include "Button.h"
#include "Entity.h"
#include "InputHandler.h"
#include "Parameters.h"
#include "Printable.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class PrintableFactory
///
/// Factory class for loading and creating printable objects from files
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PrintableFactory
{
public:
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn getFrameFromTextFile
   ///
   /// Loads a Frame from a text file. The file format is:
   /// First line: duration,layer
   /// Next N lines: ASCII art
   /// Next N lines: text RGB values (r,g,b per pixel, comma-separated, space between pixels)
   /// Next N lines: background RGB values (r,g,b per pixel, comma-separated, space between pixels)
   /// Next N lines: attribute values (integer per pixel, space between pixels)
   ///
   /// @param fileLocation - Path to the text file to load
   /// @return Frame loaded from the text file
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static Frame getFrameFromTextFile(const std::string fileLocation);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn loadAnimation
   ///
   /// @param entityName - Name of directory where all animations associated with this entity is stored
   /// @param animationName - Name of animation directory within entity directory
   /// @param repeats - Stops animation at the end of cycle or not
   /// @return Loaded Animation from inputed entityName/animationName
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static Animation loadAnimation(const std::string entityName, const std::string animationName,
                                  const bool repeats);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn loadEntity
   ///
   /// When loaded, gets stored into allPrintables global variable for display refreshing
   ///
   /// @param directoryLocation - Name of directory where all animations associated with this entity is stored
   /// @param visable - Visibility flag
   /// @param moveableByCamera - Whether the entity moves with camera
   /// @return Loaded Entity from inputed entityName
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static std::shared_ptr<Entity> loadEntity(const std::string directoryLocation, const bool visable,
                                             const bool                     moveableByCamera,
                                             std::shared_ptr<NcursesWindow> ncursesWindow = nullptr);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn loadUIElement
   ///
   /// When loaded, gets stored into allPrintables global variable for display refreshing
   ///
   /// @param directoryName - Directory name to load from
   /// @param visable - Visibility flag
   /// @param moveableByCamera - Whether the UI element moves with camera
   /// @return UIElement loaded from inputed directory name
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static std::shared_ptr<UIElement> loadUIElement(const std::string directoryName, const bool visable,
                                                   const bool                     moveableByCamera,
                                                   std::shared_ptr<NcursesWindow> ncursesWindow = nullptr);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn loadButton
   ///
   /// When loaded, gets stored into allPrintables global variable for display refreshing
   ///
   /// @param directoryName - Directory name to load from
   /// @param visable - Visibility flag
   /// @param moveableByCamera - Whether the button moves with camera
   /// @param function - The function that'll be assigned to the button when thisButton.executeFunction()
   /// @return A Button object loaded from inputed directory name
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static std::shared_ptr<Button> loadButton(const std::string directoryName, const bool visable,
                                             const bool moveableByCamera, std::function<void()> function,
                                             std::shared_ptr<NcursesWindow> ncursesWindow = nullptr);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn writePrintableToTextFiles
   ///
   /// Writes the given Printable object to .txt files in the format:
   /// src/Animations/m_printableName/m_animationName/frameN.txt
   /// Each animation is a folder, each frame is a .txt file.
   /// Overwrites existing files.
   ///
   /// File format:
   /// First line: duration,layer
   /// Next N lines: ASCII art
   /// Next N lines: text RGB values (r,g,b per pixel, comma-separated, space between pixels)
   /// Next N lines: background RGB values (r,g,b per pixel, comma-separated, space between pixels)
   /// Next N lines: attribute values (integer per pixel, space between pixels)
   ///
   /// @param printable - The Printable object to write
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static void writePrintableToTextFiles(const std::shared_ptr<Printable>& printable);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn newButton
   ///
   /// Creates a button using the default sprite, visibility = true, and moveableByCamera = false.
   ///
   /// @param text - The text to display on the button
   /// @param function - The function to execute when the button is clicked
   /// @return A new Button object
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static std::shared_ptr<Button> newButton(std::string text, std::function<void()> function,
                                            std::shared_ptr<NcursesWindow> ncursesWindow = nullptr);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn newButton (no function overload)
   ///
   /// Creates a button with no function (label only) using the default sprite, visibility = true,
   /// and moveableByCamera = false. This button cannot be clicked or highlighted.
   ///
   /// @param text - The text to display on the button
   /// @return A new Button object with no function
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static std::shared_ptr<Button> newButton(std::string                    text,
                                            std::shared_ptr<NcursesWindow> ncursesWindow = nullptr);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn newButton (template overload)
   ///
   /// Creates a button using the default sprite with a member function pointer.
   /// Automatically wraps the member function in a lambda for cleaner syntax.
   /// Usage: newButton("Click Me", &MyClass::myButtonFunc, this)
   ///
   /// @param text - The text to display on the button
   /// @param memberFunc - Pointer to the member function to call
   /// @param instance - Pointer to the class instance
   /// @return A new Button object
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<typename T>
   static std::shared_ptr<Button> newButton(std::string text, void (T::*memberFunc)(), T* instance,
                                            std::shared_ptr<NcursesWindow> ncursesWindow = nullptr)
   {
      return newButton(text, [instance, memberFunc]() { (instance->*memberFunc)(); }, ncursesWindow);
   }

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn createButtonGroup
   ///
   /// Creates multiple buttons with the same positioning and automatically sets their positions
   /// @param buttonData - Vector of pairs containing button text and function
   /// @param position - Screen position for the button group
   /// @param direction - Stacking direction for the buttons
   /// @param ncursesWindow - Window to add buttons to
   /// @return Vector of created buttons
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static std::vector<std::shared_ptr<Button>>
   createButtonGroup(const std::vector<std::pair<std::string, std::function<void()>>>& buttonData,
                     ScreenLockPosition             position      = ScreenLockPosition::CENTER,
                     StackDirection                 direction     = StackDirection::VERTICAL,
                     std::shared_ptr<NcursesWindow> ncursesWindow = nullptr);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn createSliderWithButton
   ///
   /// Creates a slider with an associated button for display/interaction
   /// @param sliderLength - Length of the slider
   /// @param buttonText - Text for the associated button
   /// @param buttonFunc - Function to call when button is clicked
   /// @param position - Screen position for the slider and button
   /// @param direction - Stacking direction for slider and button
   /// @param ncursesWindow - Window to add elements to
   /// @return Pair of button and slider
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static std::pair<std::shared_ptr<Button>, std::shared_ptr<Slider>>
   createSliderWithButton(int sliderLength, std::string buttonText, std::function<void()> buttonFunc,
                          ScreenLockPosition             position      = ScreenLockPosition::CENTER,
                          StackDirection                 direction     = StackDirection::VERTICAL,
                          std::shared_ptr<NcursesWindow> ncursesWindow = nullptr);
};

#endif