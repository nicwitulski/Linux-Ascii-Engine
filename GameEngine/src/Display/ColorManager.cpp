//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file ColorManager.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of ColorManager for color pair and RGB management in ncurses
/// @version 0.1
/// @date 2025-07-02
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../include/ColorManager.h"
#include <ncurses.h>
#include <cmath>

// Initialize static members
std::map<ColorManager::ColorKey, int> ColorManager::colorPairMap;
int                                   ColorManager::nextPair = 1; // 0 is default

// private static --------------------------------------------------------------------------------------------
// Helper: quantize 0-1000 to 0-5 (ncurses 6x6x6 cube)
static short quantize(int v)
{
   return std::round((v / 1000.0) * 5);
}

// private static --------------------------------------------------------------------------------------------
// Helper: convert quantized 0-5 to ncurses RGB (0-1000)
static short toNcursesRGB(short q)
{
   return (q * 1000) / 5;
}

// private static --------------------------------------------------------------------------------------------
ColorManager::ColorKey ColorManager::makeKey(const RGB& fg, const RGB& bg)
{
   return std::make_tuple(quantize(fg.getR()), quantize(fg.getG()), quantize(fg.getB()), quantize(bg.getR()),
                          quantize(bg.getG()), quantize(bg.getB()));
}

// private static --------------------------------------------------------------------------------------------
short ColorManager::registerColor(short r, short g, short b, short colorId)
{
   if (can_change_color())
   {
      init_color(colorId, r, g, b);
   }
   return colorId;
}

// private static --------------------------------------------------------------------------------------------
// Map quantized RGB to a color id, allocating if needed
short ColorManager::findOrRegisterColor(const RGB& rgb, short& colorIdOut)
{
   static std::map<std::tuple<short, short, short>, short> colorMap;
   static short nextColorId = 16; // 0-7: basic, 8-15: bold, 16+: custom

   auto key = std::make_tuple(quantize(rgb.getR()), quantize(rgb.getG()), quantize(rgb.getB()));
   auto it  = colorMap.find(key);
   if (it != colorMap.end())
   {
      colorIdOut = it->second;
      return colorIdOut;
   }
   if (nextColorId < COLOR_PAIRS)
   {
      colorIdOut = nextColorId++;
      registerColor(toNcursesRGB(std::get<0>(key)), toNcursesRGB(std::get<1>(key)),
                    toNcursesRGB(std::get<2>(key)), colorIdOut);
      colorMap[key] = colorIdOut;
      return colorIdOut;
   }
   colorIdOut = 0; // fallback
   return colorIdOut;
}

// public static ---------------------------------------------------------------------------------------------
int ColorManager::getColorPair(const RGB& fg, const RGB& bg)
{
   ColorKey key = makeKey(fg, bg);
   auto     it  = colorPairMap.find(key);
   if (it != colorPairMap.end())
      return it->second;

   if (nextPair >= COLOR_PAIRS)
      return 0; // fallback to default

   short fgId, bgId;
   findOrRegisterColor(fg, fgId);
   findOrRegisterColor(bg, bgId);

   init_pair(nextPair, fgId, bgId);
   colorPairMap[key] = nextPair;
   return nextPair++;
}

// public static ---------------------------------------------------------------------------------------------
void ColorManager::initialize()
{
   colorPairMap.clear();
   nextPair = 1;
}
