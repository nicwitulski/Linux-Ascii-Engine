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

#include "../../include/PrintableFactory.h"
#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>

// public static ---------------------------------------------------------------------------------------------
Frame PrintableFactory::getFrameFromTextFile(std::string fileLocation)
{
   std::ifstream inputFile(fileLocation);

   if (!inputFile.is_open())
   {
      std::cerr << "Error opening the file: " << fileLocation << std::endl;
      std::vector<Pixel> errorPixels;
      errorPixels.push_back(Pixel(Position(0, 0), '~'));
      return Frame(Sprite(errorPixels), 1.0f);
   }

   std::string        line;
   std::vector<Pixel> pixels;

   // Parse first line to get frame duration and sprite layer
   getline(inputFile, line);
   std::stringstream ss(line);
   std::string       value;
   float             duration;
   int               layer;

   if (getline(ss, value, ','))
   {
      duration = std::stof(value);
   }
   if (getline(ss, value, ','))
   {
      layer = std::stoi(value);
   }

   std::size_t x = 0;
   int         y = 0;
   while (getline(inputFile, line))
   {
      x = 0;
      while (x < line.size())
      {
         pixels.push_back(Pixel(Position(x, y), line.at(x)));
         x++;
      }
      y++;
   }
   Sprite sprite   = Sprite(pixels, layer);
   Frame  newFrame = Frame(sprite, duration);
   inputFile.close();
   return newFrame;
}

// public static ---------------------------------------------------------------------------------------------
namespace fs = std::filesystem;
Animation PrintableFactory::loadAnimation(std::string entityName, std::string animationName, bool repeats)
{
   std::string        folderPath = "src/Animations/" + entityName + "/" + animationName;
   std::vector<Frame> frames;

   try
   {
      std::vector<fs::directory_entry> entries;
      for (const auto& entry : fs::directory_iterator(folderPath))
      {
         if (entry.is_regular_file())
         {
            entries.push_back(entry);
         }
      }
      // Sort by filename
      std::sort(entries.begin(), entries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b)
                { return a.path().filename().string() < b.path().filename().string(); });

      for (const auto& entry : entries)
      {
         std::string fileName     = entry.path().filename().string();
         std::string fullFilePath = folderPath + "/" + fileName;
         std::cout << "Loading file: " << fullFilePath << std::endl;
         frames.push_back(getFrameFromTextFile(fullFilePath));
      }
      Animation animation = Animation(animationName, frames, repeats);
      if (frames.size() == 1)
      {
         animation.setPlaying(false);
      }
      return animation;
   }
   catch (const fs::filesystem_error& e)
   {
      std::cerr << "Filesystem error: " << e.what() << std::endl;
      return Animation();
   }
}

// public static ---------------------------------------------------------------------------------------------
std::shared_ptr<Entity> PrintableFactory::loadEntity(const std::string entityName, bool visable,
                                                     bool moveableByCamera)
{
   std::vector<Animation> animations;
   std::string            basePath = "src/Animations/" + entityName;

   try
   {
      for (const auto& entry : fs::directory_iterator(basePath))
      {
         if (entry.is_directory())
         {
            std::string animationName = entry.path().filename().string();
            Animation   anim          = loadAnimation(entityName, animationName, true);
            animations.push_back(anim);
         }
      }
   }
   catch (const fs::filesystem_error& e)
   {
      std::cerr << "Error loading animations for entity '" << entityName << "': " << e.what() << std::endl;
   }
   auto entity = std::make_shared<Entity>(entityName, animations, visable, moveableByCamera);
   allPrintables.push_back(entity);
   printablesNeedSorted = true;
   return entity;
}

// public static ---------------------------------------------------------------------------------------------
std::shared_ptr<UIElement> PrintableFactory::loadUIElement(const std::string directoryName,
                                                           const bool visable, const bool moveableByCamera)

{
   std::vector<Animation> animations;
   std::string            basePath = "src/Animations/" + directoryName;

   try
   {
      for (const auto& entry : fs::directory_iterator(basePath))
      {
         if (entry.is_directory())
         {
            std::string animationName = entry.path().filename().string();
            Animation   anim          = loadAnimation(directoryName, animationName, true);
            animations.push_back(anim);
         }
      }
   }
   catch (const fs::filesystem_error& e)
   {
      std::cerr << "Error loading animations for entity '" << directoryName << "': " << e.what() << std::endl;
   }
   auto uiElement = std::make_shared<UIElement>(animations, visable, moveableByCamera);
   allPrintables.push_back(uiElement);
   printablesNeedSorted = true;
   return uiElement;
};

// public static ---------------------------------------------------------------------------------------------
std::shared_ptr<Button> PrintableFactory::loadButton(const std::string directoryName, const bool visable,
                                                     const bool            moveableByCamera,
                                                     std::function<void()> function)
{
   std::vector<Animation> animations;
   std::string            basePath = "src/Animations/" + directoryName;

   try
   {
      for (const auto& entry : fs::directory_iterator(basePath))
      {
         if (entry.is_directory())
         {
            std::string animationName = entry.path().filename().string();
            Animation   anim          = loadAnimation(directoryName, animationName, true);
            animations.push_back(anim);
         }
      }
   }
   catch (const fs::filesystem_error& e)
   {
      std::cerr << "Error loading animations for entity '" << directoryName << "': " << e.what() << std::endl;
   }
   auto button = std::make_shared<Button>(animations, visable, moveableByCamera, function);
   allPrintables.push_back(button);
   printablesNeedSorted = true;
   return button;
}