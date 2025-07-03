//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file FileLoading.h
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief File used to load objects from .txt files
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
#include "Parameters.h"
#include "Printable.h"
#include "InputHandler.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class PrintableFactory
///
/// File used to load objects from .txt files
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
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static Frame getFrameFromTextFile(const std::string fileLocation);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn loadAnimation
   ///
   /// @return loaded Animation from inputed entityName/animationName
   /// @param entityName - Name of directory where all animations associated with this entity is stored
   /// @param animationName - Name of animation directory within entity directory
   /// @param repeats - Stops animation at the end of cycle or not
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static Animation loadAnimation(const std::string entityName, const std::string animationName,
                                  const bool repeats);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn loadEntity
   ///
   /// When loaded, gets stored into allPrintables global variable for display refreshing
   ///
   /// @return loaded Entity from inputed entityName
   /// @param entityName - Name of directory where all animations associated with this entity is stored
   /// @param visable
   /// @param layer
   /// @param movemableByCamera
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static std::shared_ptr<Entity> loadEntity(const std::string directoryLocation, const bool visable,
                                             const bool moveableByCamera);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn loadUIElement
   ///
   /// When loaded, gets stored into allPrintables global variable for display refreshing
   ///
   /// @return UIElement loaded from inputed directory name. Similar to entityName but doesn't actually get
   /// assigned to the object
   /// @param directoryName
   /// @param visable
   /// @param layer
   /// @param moveableByCamera
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static std::shared_ptr<UIElement> loadUIElement(const std::string directoryName, const bool visable,
                                                   const bool moveableByCamera);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn loadButton
   ///
   /// When loaded, gets stored into allPrintables global variable for display refreshing
   ///
   /// @return a Button object loaded from inputed directory name
   /// @param directoryName
   /// @param visable
   /// @param layer
   /// @param moveableByCamera
   /// @param function - The function that'll be assigned to the button when thisButton.executeFuntion()
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static std::shared_ptr<Button> loadButton(const std::string directoryName, const bool visable,
                                             const bool moveableByCamera, std::function<void()> function);

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
   /// @param printable - the Printable object to write
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static void writePrintableToTextFiles(const std::shared_ptr<Printable>& printable);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn writePrintableToTextFiles
   ///
   /// Creates a button using the default sprite, visability = true, and moveableByCamera = false.
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static std::shared_ptr<Button> newButton(std::string text,std::function<void()> function);

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
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<typename T>
   static std::shared_ptr<Button> newButton(std::string text, void (T::*memberFunc)(), T* instance)
   {
      return newButton(text, [instance, memberFunc]() { (instance->*memberFunc)(); });
   }
};

#endif