//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Display.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of Display class for ncurses display management
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../include/Display.h"
#include "../../include/ColorManager.h"
#include "../../include/Parameters.h"
#include "../../include/UIElement.h"
#include <ncursesw/ncurses.h>
#include <cwchar>

// public static ---------------------------------------------------------------------------------------------
void Display::removeWindow(std::shared_ptr<NcursesWindow> window)
{
   ncursesWindows.erase(std::remove(ncursesWindows.begin(), ncursesWindows.end(), window),
                        ncursesWindows.end());
   ::ncursesWindows.erase(std::remove(::ncursesWindows.begin(), ::ncursesWindows.end(), window),
                          ::ncursesWindows.end());
}

// public static ---------------------------------------------------------------------------------------------
void Display::initCurse()
{
   setlocale(LC_ALL, "");
   initscr(); // Start curses mode
   refresh();
   curs_set(0);
   noecho();
   cbreak();
   keypad(stdscr, TRUE);
   nodelay(stdscr, TRUE);
   mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
   mouseinterval(0);
   printf("\033[?1003h\n"); // Enable mouse tracking

   getmaxyx(stdscr, SCREEN_HEIGHT, SCREEN_LENGTH);

   auto stdWindow = std::make_shared<NcursesWindow>(stdscr, 0, false);
   stdWindow->setBorderEnabled(false);
   ncursesWindows.push_back(stdWindow);

   for (auto& window : ncursesWindows)
   {
      window->clearBuffer();
   }

   start_color();
   use_default_colors();
   ColorManager::initialize();
   UIElement::updateAllLockedPositions();
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

// private static --------------------------------------------------------------------------------------------
void refreshWindowRecursively(std::shared_ptr<NcursesWindow> window, float deltaTime)
{
   // Refresh this window
   window->refreshWindow(deltaTime);

   // Sort and refresh all sub-windows by layer
   auto subWindows = window->getSubWindows();
   std::sort(subWindows.begin(), subWindows.end(),
             [](const std::shared_ptr<NcursesWindow>& a, const std::shared_ptr<NcursesWindow>& b)
             { return a->getWindowLayer() < b->getWindowLayer(); });

   for (auto& subWindow : subWindows)
   {
      refreshWindowRecursively(subWindow, deltaTime);
   }
}

// public static ---------------------------------------------------------------------------------------------
void Display::refreshDisplay(float deltaTime)
{
   // Sort top-level windows by layer (excludes sub-windows)
   auto topLevelWindows = ncursesWindows;
   topLevelWindows.erase(std::remove_if(topLevelWindows.begin(), topLevelWindows.end(),
                                        [](const std::shared_ptr<NcursesWindow>& window)
                                        { return window->isSubWindow(); }),
                         topLevelWindows.end());

   std::sort(topLevelWindows.begin(), topLevelWindows.end(),
             [](const std::shared_ptr<NcursesWindow>& a, const std::shared_ptr<NcursesWindow>& b)
             { return a->getWindowLayer() < b->getWindowLayer(); });

   // Refresh all top-level windows and their sub-windows recursively
   for (auto& window : topLevelWindows)
   {
      refreshWindowRecursively(window, deltaTime);
   }

   // Update screen once after all windows have been refreshed
   doupdate();

   // Reset global display clear flag after all windows have been processed
   displayNeedsCleared = false;
}