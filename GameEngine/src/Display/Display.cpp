//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Display.h
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Handles all of the ncurses calls, prints, and refreshes
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../include/Display.h" 
#include "../../include/ColorManager.h"
#include <ncurses.h>
#include <cwchar>

// Initialize static members
std::vector<std::vector<Pixel>> Display::currentFrameBuffer;
std::vector<std::vector<Pixel>> Display::lastFrameBuffer;

// public static ---------------------------------------------------------------------------------------------
void Display::initCurse()
{
   setlocale(LC_ALL, "");
   initscr(); // Start curses mode
   curs_set(0);
   noecho();
   cbreak();
   keypad(stdscr, TRUE);
   nodelay(stdscr, TRUE);
   mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
   mouseinterval(0);
   printf("\033[?1003h\n"); // Enable mouse tracking

   getmaxyx(stdscr, SCREEN_HEIGHT, SCREEN_LENGTH);
   currentFrameBuffer =
         std::vector<std::vector<Pixel>>(SCREEN_HEIGHT, std::vector<Pixel>(SCREEN_LENGTH, Pixel()));
   lastFrameBuffer = currentFrameBuffer;

   start_color();
   use_default_colors();
   ColorManager::initialize();
}

// public static ---------------------------------------------------------------------------------------------
char Display::getUserInput()
{
   return getch();
}

// public static ---------------------------------------------------------------------------------------------
void Display::closeCurseWindow()
{
   printf("\033[?1003l\n"); // Disable mouse tracking
   endwin();
}

// public static ---------------------------------------------------------------------------------------------
void Display::refreshDisplay(float deltaTime)
{
   int tempHeight, tempLength;
   getmaxyx(stdscr, tempHeight, tempLength);

   if (tempHeight != SCREEN_HEIGHT || tempLength != SCREEN_LENGTH)
   {
      SCREEN_HEIGHT = tempHeight;
      SCREEN_LENGTH = tempLength;

      currentCamera->setHeight(SCREEN_HEIGHT);
      currentCamera->setLength(SCREEN_LENGTH);

      UIElement::updateAllLockedPositions();

      currentFrameBuffer =
            std::vector<std::vector<Pixel>>(SCREEN_HEIGHT, std::vector<Pixel>(SCREEN_LENGTH, Pixel()));
      lastFrameBuffer     = currentFrameBuffer;
      displayNeedsCleared = true;
   }

   if (displayNeedsCleared)
   {
      clear();
      for (auto& row : currentFrameBuffer)
         std::fill(row.begin(), row.end(), Pixel());
      for (auto& row : lastFrameBuffer)
         std::fill(row.begin(), row.end(), Pixel());
      displayNeedsCleared = false;
   }

   refreshEntities(deltaTime);

   // Draw diffs
   for (int y = 0; y < SCREEN_HEIGHT; ++y)
   {
      for (int x = 0; x < SCREEN_LENGTH; ++x)
      {
         const Pixel& curr = currentFrameBuffer[y][x];
         const Pixel& last = lastFrameBuffer[y][x];
         if (curr.getCharacter() != last.getCharacter() ||
             curr.getTextColor().getR() != last.getTextColor().getR() ||
             curr.getTextColor().getG() != last.getTextColor().getG() ||
             curr.getTextColor().getB() != last.getTextColor().getB() ||
             curr.getBackgroundColor().getR() != last.getBackgroundColor().getR() ||
             curr.getBackgroundColor().getG() != last.getBackgroundColor().getG() ||
             curr.getBackgroundColor().getB() != last.getBackgroundColor().getB() ||
             curr.getAttributes() != last.getAttributes())
         {
            attr_t attr      = curr.getAttributes();
            RGB    fg        = curr.getTextColor();
            RGB    bg        = curr.getBackgroundColor();
            int    colorPair = ColorManager::getColorPair(fg, bg);

            attrset(attr);
            if (has_colors())
            {
               color_set(colorPair, NULL);
            }
            
            // Use wide-character function for Unicode support
            cchar_t wch;
            wch.chars[0] = curr.getCharacter();
            wch.chars[1] = L'\0';
            wch.attr = attr;
            wch.ext_color = colorPair;
            mvadd_wch(y, x, &wch);
            
            attroff(attr);
            if (has_colors())
            {
               color_set(0, NULL); // Reset to default after drawing
            }

            lastFrameBuffer[y][x] = curr;
         }
      }
   }

   wnoutrefresh(stdscr);
   doupdate();
   curs_set(0);
}

// public static ---------------------------------------------------------------------------------------------
void Display::printPixel(const Pixel pixel, bool isMoveableByCamera)
{
   int printedX = pixel.getPosition().getX();
   int printedY = pixel.getPosition().getY();

   if (isMoveableByCamera)
   {
      printedX += currentCamera->getLengthOffset();
      printedY += currentCamera->getHeightOffset();
   }

   if (printedX >= 0 && printedX < SCREEN_LENGTH && printedY >= 0 && printedY < SCREEN_HEIGHT)
   {
      currentFrameBuffer[printedY][printedX] = pixel;
   }
}

// public static ---------------------------------------------------------------------------------------------
void Display::printSprite(Sprite sprite, bool isMoveableByCamera)
{
   for (const Pixel& pixel : sprite.getPixels())
   {
      printPixel(pixel, isMoveableByCamera);
   }
}

// public static ---------------------------------------------------------------------------------------------
void Display::eraseSprite(Sprite sprite, bool isMoveableByCamera)
{
   for (const Pixel& pixel : sprite.getPixels())
   {
      printPixel(Pixel(pixel.getPosition(), ' '), isMoveableByCamera);
   }
}

// public static ---------------------------------------------------------------------------------------------
void Display::refreshEntities(float deltaTime)
{
   if (printablesNeedSorted)
   {
      std::sort(allPrintables.begin(), allPrintables.end(),
                [](const std::shared_ptr<Printable>& a, const std::shared_ptr<Printable>& b)
                {
                   return a->getCurrentAnimation().getCurrentFrameSprite().getLayer() <
                          b->getCurrentAnimation().getCurrentFrameSprite().getLayer();
                });
      printablesNeedSorted = false;
   }

   for (auto& printable : allPrintables)
   {
      for (Animation& animation : printable->getAnimationsMutable())
      {
         if (animation.getAnimationName() == printable->getCurrentAnimationName())
         {
            if (animation.isPlaying())
            {
               animation.update(deltaTime);
               printable->addDirtySprite(animation.getPreviousFrameSprite());
            }

            for (const Sprite& sprite : printable->getDirtySprites())
            {
               eraseSprite(sprite, printable->isMoveableByCamera());
            }
            printable->clearDirtySprites();
            printSprite(animation.getCurrentFrameSprite(), printable->isMoveableByCamera());
         }
      }
   }
}