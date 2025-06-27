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
#include <filesystem>
#include <fstream>
#include <iostream>
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
   /// @return loaded Frame from inputed fileLocation
   /// @param fileLocation - fileLocation string from view of User Application root directory
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
   static std::shared_ptr<Entity> loadEntity(const std::string entityName, const bool visable,
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
};

#endif