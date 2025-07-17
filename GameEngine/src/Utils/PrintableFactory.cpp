//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file PrintableFactory.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of PrintableFactory for loading and creating printable objects from files
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../include/PrintableFactory.h"
#include <algorithm>
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <memory>
#include <sstream> // for stringstream
#include <string>

namespace fs = std::filesystem;

// public static ---------------------------------------------------------------------------------------------
Frame PrintableFactory::getFrameFromTextFile(const std::string fileLocation)
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
   float             duration = 1.0f;
   int               layer    = 0;

   if (getline(ss, value, ','))
   {
      duration = std::stof(value);
   }
   if (getline(ss, value, ','))
   {
      layer = std::stoi(value);
   }

   // Read ASCII art lines until delimiter '---' or end of file
   std::vector<std::string> asciiLines;
   bool                     foundDelimiter = false;
   while (getline(inputFile, line))
   {
      if (line == "---")
      {
         foundDelimiter = true;
         break;
      }
      asciiLines.push_back(line);
   }

   int height = asciiLines.size();
   int width  = 0;
   for (const auto& l : asciiLines)
      if ((int)l.size() > width)
         width = l.size();

   // Default color values (white text, black background, normal attributes)
   std::vector<std::vector<RGB>>    textRGBs(height, std::vector<RGB>(width, RGB(1000, 1000, 1000)));
   std::vector<std::vector<RGB>>    bgRGBs(height, std::vector<RGB>(width, RGB(0, 0, 0)));
   std::vector<std::vector<attr_t>> attrs(height, std::vector<attr_t>(width, A_NORMAL));

   // Only read color/attribute data if delimiter was found
   if (foundDelimiter)
   {
      // Read text RGB lines
      for (int y = 0; y < height; ++y)
      {
         if (!getline(inputFile, line))
            break;
         std::stringstream ss(line);
         for (int x = 0; x < width; ++x)
         {
            std::string pixelStr;
            if (!(ss >> pixelStr))
               break;
            int    r = 1000, g = 1000, b = 1000;
            size_t p1 = pixelStr.find(',');
            size_t p2 = pixelStr.find(',', p1 + 1);
            if (p1 != std::string::npos && p2 != std::string::npos)
            {
               r = std::stoi(pixelStr.substr(0, p1));
               g = std::stoi(pixelStr.substr(p1 + 1, p2 - p1 - 1));
               b = std::stoi(pixelStr.substr(p2 + 1));
            }
            textRGBs[y][x] = RGB(r, g, b);
         }
      }

      // Read background RGB lines
      for (int y = 0; y < height; ++y)
      {
         if (!getline(inputFile, line))
            break;
         std::stringstream ss(line);
         for (int x = 0; x < width; ++x)
         {
            std::string pixelStr;
            if (!(ss >> pixelStr))
               break;
            int    r = 0, g = 0, b = 0;
            size_t p1 = pixelStr.find(',');
            size_t p2 = pixelStr.find(',', p1 + 1);
            if (p1 != std::string::npos && p2 != std::string::npos)
            {
               r = std::stoi(pixelStr.substr(0, p1));
               g = std::stoi(pixelStr.substr(p1 + 1, p2 - p1 - 1));
               b = std::stoi(pixelStr.substr(p2 + 1));
            }
            bgRGBs[y][x] = RGB(r, g, b);
         }
      }

      // Read attribute lines
      for (int y = 0; y < height; ++y)
      {
         if (!getline(inputFile, line))
            break;
         std::stringstream ss(line);
         for (int x = 0; x < width; ++x)
         {
            int attr = 0;
            if (!(ss >> attr))
               break;
            attrs[y][x] = static_cast<attr_t>(attr);
         }
      }
   }

   // Compose pixels
   std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
   for (int y = 0; y < height; ++y)
   {
      std::wstring wline = converter.from_bytes(asciiLines[y]);
      for (int x = 0; x < (int)wline.size(); ++x)
      {
         wchar_t ch        = wline[x];
         RGB     textColor = textRGBs[y][x];
         RGB     bgColor   = bgRGBs[y][x];
         attr_t  attr      = attrs[y][x];
         pixels.push_back(Pixel(Position(x, y), ch, textColor, bgColor, attr));
      }
   }
   Sprite sprite   = Sprite(pixels, layer);
   Frame  newFrame = Frame(sprite, duration);
   inputFile.close();
   return newFrame;
}

// public static ---------------------------------------------------------------------------------------------
Animation PrintableFactory::loadAnimation(std::string entityName, std::string animationName, bool repeats)
{
   namespace fs                  = std::filesystem;
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
                                                     bool                           moveableByCamera,
                                                     std::shared_ptr<NcursesWindow> ncursesWindow)
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

   if (ncursesWindow != nullptr)
   {
      entity->setNcurseWindow(ncursesWindow->getWindow());
      ncursesWindow->addPrintable(entity);
      ncursesWindow->setPrintablesNeedSorted(true);
   }
   else if (!ncursesWindows.empty())
   {
      entity->setNcurseWindow(ncursesWindows.at(0)->getWindow());
      ncursesWindows.at(0)->addPrintable(entity);
      ncursesWindows.at(0)->setPrintablesNeedSorted(true);
   }
   else
   {
      std::cerr << "Warning: No ncurses windows available for entity '" << entityName << "'" << std::endl;
   }
   return entity;
}

// public static ---------------------------------------------------------------------------------------------
std::shared_ptr<UIElement> PrintableFactory::loadUIElement(const std::string directoryName,
                                                           const bool visable, const bool moveableByCamera,
                                                           std::shared_ptr<NcursesWindow> ncursesWindow)

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
   auto uiElement = std::make_shared<UIElement>(directoryName, animations, visable, moveableByCamera);
   if (ncursesWindow != nullptr)
   {
      uiElement->setNcurseWindow(ncursesWindow->getWindow());
      ncursesWindow->addPrintable(uiElement);
      ncursesWindow->setPrintablesNeedSorted(true);
   }
   else if (!ncursesWindows.empty())
   {
      uiElement->setNcurseWindow(ncursesWindows.at(0)->getWindow());
      ncursesWindows.at(0)->addPrintable(uiElement);
      ncursesWindows.at(0)->setPrintablesNeedSorted(true);
   }
   else
   {
      std::cerr << "Warning: No ncurses windows available for UIElement '" << directoryName << "'" << std::endl;
   }
   return uiElement;
};

// public static ---------------------------------------------------------------------------------------------
std::shared_ptr<Button>
PrintableFactory::loadButton(const std::string directoryName, const bool visable, const bool moveableByCamera,
                             std::function<void()> function, std::shared_ptr<NcursesWindow> ncursesWindow)
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
   auto button = std::make_shared<Button>(directoryName, animations, visable, moveableByCamera, function);
   if (ncursesWindow != nullptr)
   {
      button->setNcurseWindow(ncursesWindow->getWindow());
      ncursesWindow->addPrintable(button);
      ncursesWindow->setPrintablesNeedSorted(true);
   }
   else if (!ncursesWindows.empty())
   {
      button->setNcurseWindow(ncursesWindows.at(0)->getWindow());
      ncursesWindows.at(0)->addPrintable(button);
      ncursesWindows.at(0)->setPrintablesNeedSorted(true);
   }
   else
   {
      std::cerr << "Warning: No ncurses windows available for Button '" << directoryName << "'" << std::endl;
   }

   // Automatically register with global InputHandler
   globalInputHandler.addButton(button);

   return button;
}

// public static ---------------------------------------------------------------------------------------------
std::shared_ptr<Button> PrintableFactory::newButton(std::string text, std::function<void()> function,
                                                    std::shared_ptr<NcursesWindow> ncursesWindow)
{
   // Load the default button sprite
   std::vector<Animation> animations;
   std::string            basePath = "src/Animations/defaultBorder";

   try
   {
      for (const auto& entry : fs::directory_iterator(basePath))
      {
         if (entry.is_directory())
         {
            std::string animationName = entry.path().filename().string();
            Animation   anim          = loadAnimation("defaultBorder", animationName, true);
            animations.push_back(anim);
         }
      }
   }
   catch (const fs::filesystem_error& e)
   {
      std::cerr << "Error loading default button animations: " << e.what() << std::endl;
      // Create a fallback button if loading fails
      std::vector<Pixel> fallbackPixels = {Pixel(Position(0, 0), L'+'), Pixel(Position(1, 0), L'-'),
                                           Pixel(Position(2, 0), L'-'), Pixel(Position(3, 0), L'-'),
                                           Pixel(Position(4, 0), L'-'), Pixel(Position(5, 0), L'+'),
                                           Pixel(Position(0, 1), L'|'), Pixel(Position(1, 1), L' '),
                                           Pixel(Position(2, 1), L' '), Pixel(Position(3, 1), L' '),
                                           Pixel(Position(4, 1), L' '), Pixel(Position(5, 1), L'|'),
                                           Pixel(Position(0, 2), L'+'), Pixel(Position(1, 2), L'-'),
                                           Pixel(Position(2, 2), L'-'), Pixel(Position(3, 2), L'-'),
                                           Pixel(Position(4, 2), L'-'), Pixel(Position(5, 2), L'+')};
      Sprite             fallbackSprite(fallbackPixels, 1);
      Frame              fallbackFrame(fallbackSprite, 10);
      std::vector<Frame> fallbackFrames = {fallbackFrame};
      Animation          fallbackAnim("default", fallbackFrames, true);
      animations.push_back(fallbackAnim);
   }

   // Create the button
   auto button = std::make_shared<Button>("defaultBorder", animations, true, false, function);

   // Set the text (this will wrap the outline around the text)
   button->setText(text);

   // Add to printables list
   if (ncursesWindow != nullptr)
   {
      button->setNcurseWindow(ncursesWindow->getWindow());
      ncursesWindow->addPrintable(button);
      ncursesWindow->setPrintablesNeedSorted(true);
   }
   else if (!ncursesWindows.empty())
   {
      button->setNcurseWindow(ncursesWindows.at(0)->getWindow());
      ncursesWindows.at(0)->addPrintable(button);
      ncursesWindows.at(0)->setPrintablesNeedSorted(true);
   }
   else
   {
      std::cerr << "Warning: No ncurses windows available for Button with text '" << text << "'" << std::endl;
   }

   // Automatically register with global InputHandler
   globalInputHandler.addButton(button);

   return button;
}

// public static ---------------------------------------------------------------------------------------------
void PrintableFactory::writePrintableToTextFiles(const std::shared_ptr<Printable>& printable)
{
   namespace fs = std::filesystem;
   if (!printable)
      return;

   std::string baseDir = "src/Animations/" + printable->getPrintableName();
   fs::create_directories(baseDir);

   const auto& animations = printable->getAnimations();
   for (const auto& animation : animations)
   {
      std::string animationDir = baseDir + "/" + animation.getAnimationName();
      fs::create_directories(animationDir);

      const auto& frames = animation.getFrames();
      for (size_t i = 0; i < frames.size(); ++i)
      {
         const auto&   frame  = frames[i];
         const Sprite& sprite = frame.getSprite();
         const auto&   pixels = sprite.getPixels();

         // Find max X and Y for frame size
         int maxX = 0, maxY = 0;
         for (const auto& pixel : pixels)
         {
            int x = pixel.getPosition().getX();
            int y = pixel.getPosition().getY();
            if (x > maxX)
               maxX = x;
            if (y > maxY)
               maxY = y;
         }

         // Prepare 2D arrays for output
         std::vector<std::string>      asciiLines(maxY + 1, std::string(maxX + 1, ' '));
         std::vector<std::vector<RGB>> textRGBs(maxY + 1, std::vector<RGB>(maxX + 1, RGB(1000, 1000, 1000)));
         std::vector<std::vector<RGB>> bgRGBs(maxY + 1, std::vector<RGB>(maxX + 1, RGB(0, 0, 0)));
         std::vector<std::vector<attr_t>> attrs(maxY + 1, std::vector<attr_t>(maxX + 1, A_NORMAL));
         for (const auto& pixel : pixels)
         {
            int x = pixel.getPosition().getX();
            int y = pixel.getPosition().getY();
            if (y >= 0 && y < (int)asciiLines.size() && x >= 0 && x < (int)asciiLines[y].size())
            {
               asciiLines[y][x] = pixel.getCharacter();
               textRGBs[y][x]   = pixel.getTextColor();
               bgRGBs[y][x]     = pixel.getBackgroundColor();
               attrs[y][x]      = pixel.getAttributes();
            }
         }

         // Write to file
         std::string   frameFile = animationDir + "/frame" + std::to_string(i) + ".txt";
         std::ofstream out(frameFile, std::ios::trunc);
         if (!out.is_open())
         {
            std::cerr << "Failed to write frame file: " << frameFile << std::endl;
            continue;
         }
         // Write duration and layer as first line
         out << frame.getDuration() << "," << sprite.getLayer() << "\n";
         // Write sprite lines
         for (const auto& line : asciiLines)
            out << line << "\n";
         // Write delimiter
         out << "---\n";
         // Write text RGB lines
         for (const auto& row : textRGBs)
         {
            for (size_t x = 0; x < row.size(); ++x)
            {
               out << row[x].getR() << "," << row[x].getG() << "," << row[x].getB();
               if (x < row.size() - 1)
                  out << " ";
            }
            out << "\n";
         }
         // Write background RGB lines
         for (const auto& row : bgRGBs)
         {
            for (size_t x = 0; x < row.size(); ++x)
            {
               out << row[x].getR() << "," << row[x].getG() << "," << row[x].getB();
               if (x < row.size() - 1)
                  out << " ";
            }
            out << "\n";
         }
         // Write attribute lines
         for (const auto& row : attrs)
         {
            for (size_t x = 0; x < row.size(); ++x)
            {
               out << static_cast<int>(row[x]);
               if (x < row.size() - 1)
                  out << " ";
            }
            out << "\n";
         }
         out.close();
      }
   }
}

// public static ---------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<Button>> PrintableFactory::createButtonGroup(
      const std::vector<std::pair<std::string, std::function<void()>>>& buttonData,
      ScreenLockPosition position, StackDirection direction, std::shared_ptr<NcursesWindow> ncursesWindow)
{
   std::vector<std::shared_ptr<Button>> buttons;

   for (const auto& data : buttonData)
   {
      auto button = newButton(data.first, data.second, ncursesWindow);
      button->setDynamicPosition(position, direction);
      buttons.push_back(button);
   }

   return buttons;
}

// public static ---------------------------------------------------------------------------------------------
std::pair<std::shared_ptr<Button>, std::shared_ptr<Slider>>
PrintableFactory::createSliderWithButton(int sliderLength, std::string buttonText,
                                         std::function<void()> buttonFunc, ScreenLockPosition position,
                                         StackDirection direction, std::shared_ptr<NcursesWindow> ncursesWindow)
{
   auto button = newButton(buttonText, buttonFunc, ncursesWindow);
   auto slider = std::make_shared<Slider>(sliderLength, true);

   button->setDynamicPosition(position, direction);
   slider->setDynamicPosition(position, direction);
   slider->setVisability(true);
   slider->setMoveableByCamera(false);
   slider->setAllAnimationSpriteLayers(1);

   if (ncursesWindow)
   {
      slider->setNcurseWindow(ncursesWindow->getWindow());
      ncursesWindow->addPrintable(slider);
   }
   else if (!ncursesWindows.empty())
   {
      slider->setNcurseWindow(ncursesWindows.at(0)->getWindow());
      ncursesWindows.at(0)->addPrintable(slider);
   }

   globalInputHandler.addSlider(slider);

   return std::make_pair(button, slider);
}