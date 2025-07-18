//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file UIElement.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of UIElement class with dynamic positioning capabilities
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/UIElement.h"
#include "../../../include/Parameters.h"
#include <algorithm>
#include <climits>
#include <map>
#include <set>

// Initialize static member variables
std::vector<std::shared_ptr<UIElement>> UIElement::topMiddleUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::rightMiddleUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::bottomMiddleUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::leftMiddleUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::middleUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::topLeftUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::topRightUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::bottomLeftUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::bottomRightUIElements;

// public ----------------------------------------------------------------------------------------------------
UIElement::UIElement()
{
   m_printableName = "default";
   m_minPosition   = Position(0, 0);
   m_maxPosition   = Position(0, 0);
   m_lockPosition  = ScreenLockPosition::NONE;

   m_currentAnimationName = "default";
   m_animations.push_back(Animation());
   m_visable          = false;
   m_moveableByCamera = true;
   m_ncurseWindow     = nullptr;

   // Initialize border properties
   m_borderEnabled  = false;
   m_borderWidth    = 0;
   m_borderHeight   = 0;
   m_borderAutoSize = true;
};

// public ----------------------------------------------------------------------------------------------------
UIElement::UIElement(const std::string printableName, const std::vector<Animation> animations,
                     const bool visable, const bool moveableByCamera)
{
   m_printableName        = printableName;
   m_animations           = animations;
   m_currentAnimationName = m_animations.at(0).getAnimationName();
   m_visable              = visable;
   m_moveableByCamera     = moveableByCamera;
   m_lockPosition         = ScreenLockPosition::NONE;
   m_ncurseWindow         = nullptr;

   // Initialize border properties
   m_borderEnabled  = false;
   m_borderWidth    = 0;
   m_borderHeight   = 0;
   m_borderAutoSize = true;

   setPositions();
};

// private ---------------------------------------------------------------------------------------------------
void UIElement::setPositions()
{
   m_minPosition = getCurrentAnimation().getCurrentFrameSprite().getAnchor();
   int maxX      = 0;
   int maxY      = 0;

   for (Animation animation : getAnimations())
   {
      if (animation.getAnimationName() == getCurrentAnimationName())
      {
         for (const Pixel& pixel : animation.getCurrentFrameSprite().getPixels())
         {
            if (pixel.getPosition().getX() > maxX)
               maxX = pixel.getPosition().getX();
            if (pixel.getPosition().getY() > maxY)
               maxY = pixel.getPosition().getY();
         }
      }
   }

   m_maxPosition = Position(maxX, maxY);
};

// public ----------------------------------------------------------------------------------------------------
void UIElement::setDynamicPosition(ScreenLockPosition position, StackDirection direction)
{
   // First, clean up any expired elements to prevent accumulation
   cleanupExpiredElements();

   m_lockPosition   = position;
   m_stackDirection = direction;

   // Helper function to check if element is already in vector
   auto addIfNotPresent = [this](std::vector<std::shared_ptr<UIElement>>& vec)
   {
      auto thisPtr = shared_from_this();
      if (std::find(vec.begin(), vec.end(), thisPtr) == vec.end())
      {
         vec.push_back(thisPtr);
      }
   };

   switch (m_lockPosition)
   {
      case ScreenLockPosition::TOP_MIDDLE:
         addIfNotPresent(topMiddleUIElements);
         break;
      case ScreenLockPosition::RIGHT_MIDDLE:
         addIfNotPresent(rightMiddleUIElements);
         break;
      case ScreenLockPosition::BOTTOM_MIDDLE:
         addIfNotPresent(bottomMiddleUIElements);
         break;
      case ScreenLockPosition::LEFT_MIDDLE:
         addIfNotPresent(leftMiddleUIElements);
         break;
      case ScreenLockPosition::CENTER:
         addIfNotPresent(middleUIElements);
         break;
      case ScreenLockPosition::TOP_LEFT_CORNER:
         addIfNotPresent(topLeftUIElements);
         break;
      case ScreenLockPosition::TOP_RIGHT_CORNER:
         addIfNotPresent(topRightUIElements);
         break;
      case ScreenLockPosition::BOTTOM_LEFT_CORNER:
         addIfNotPresent(bottomLeftUIElements);
         break;
      case ScreenLockPosition::BOTTOM_RIGHT_CORNER:
         addIfNotPresent(bottomRightUIElements);
         break;

      default:
         break;
   }
   m_stackDirection = direction;
};

// public ----------------------------------------------------------------------------------------------------
void UIElement::updateWindowLockedPositions(WINDOW* targetWindow)
{
   // Helper lambdas
   auto elementWidth = [](const std::shared_ptr<UIElement>& el)
   { return el->m_maxPosition.getX() - el->m_minPosition.getX() + 1; };
   auto elementHeight = [](const std::shared_ptr<UIElement>& el)
   { return el->m_maxPosition.getY() - el->m_minPosition.getY() + 1; };

   // Helper function to find NcursesWindow from WINDOW*
   auto findNcursesWindow = [](WINDOW* window) -> std::shared_ptr<NcursesWindow>
   {
      for (auto& ncursesWindow : ncursesWindows)
      {
         if (ncursesWindow->getWindow() == window)
         {
            return ncursesWindow;
         }
      }
      return nullptr;
   };

   // Filter elements to only process those associated with the target window
   auto filterWindowElements = [targetWindow](const std::vector<std::shared_ptr<UIElement>>& elements)
   {
      std::vector<std::shared_ptr<UIElement>> filtered;
      for (auto& el : elements)
      {
         WINDOW* window = el->getNcurseWindow();
         if ((!window && targetWindow == stdscr) || window == targetWindow)
         {
            filtered.push_back(el);
         }
      }
      return filtered;
   };

   // Filter each position group to only include target window elements
   auto filteredTopMiddle    = filterWindowElements(topMiddleUIElements);
   auto filteredRightMiddle  = filterWindowElements(rightMiddleUIElements);
   auto filteredBottomMiddle = filterWindowElements(bottomMiddleUIElements);
   auto filteredLeftMiddle   = filterWindowElements(leftMiddleUIElements);
   auto filteredMiddle       = filterWindowElements(middleUIElements);
   auto filteredTopLeft      = filterWindowElements(topLeftUIElements);
   auto filteredTopRight     = filterWindowElements(topRightUIElements);
   auto filteredBottomLeft   = filterWindowElements(bottomLeftUIElements);
   auto filteredBottomRight  = filterWindowElements(bottomRightUIElements);

   // Group filtered elements by their window
   std::map<WINDOW*, std::vector<std::shared_ptr<UIElement>>> windowGroups[9];

   // Helper function to add elements to window groups
   auto addToWindowGroup = [&](const std::vector<std::shared_ptr<UIElement>>& elements, int groupIndex)
   {
      for (auto& el : elements)
      {
         WINDOW* window = el->getNcurseWindow();
         if (!window)
            window = stdscr;
         windowGroups[groupIndex][window].push_back(el);
      }
   };

   // Group all filtered elements by window and position
   addToWindowGroup(filteredTopMiddle, 0);
   addToWindowGroup(filteredRightMiddle, 1);
   addToWindowGroup(filteredBottomMiddle, 2);
   addToWindowGroup(filteredLeftMiddle, 3);
   addToWindowGroup(filteredMiddle, 4);
   addToWindowGroup(filteredTopLeft, 5);
   addToWindowGroup(filteredTopRight, 6);
   addToWindowGroup(filteredBottomLeft, 7);
   addToWindowGroup(filteredBottomRight, 8);

   // Process each window
   for (int groupIndex = 0; groupIndex < 9; ++groupIndex)
   {
      for (auto& [window, elements] : windowGroups[groupIndex])
      {
         if (elements.empty())
            continue;

         // Get window dimensions and adjust for borders if enabled
         int windowHeight, windowLength;
         getmaxyx(window, windowHeight, windowLength);

         // Check if this window has borders and adjust usable area
         auto ncursesWindow = findNcursesWindow(window);
         int  borderPadding = 0;
         if (ncursesWindow && ncursesWindow->isBorderEnabled())
         {
            borderPadding = 1; // 1 character padding on each side for borders
            windowHeight -= 2; // Top and bottom border
            windowLength -= 2; // Left and right border
         }

         // Process elements based on their position type
         switch (groupIndex)
         {
            case 0: // TOP MIDDLE
            {
               int totalWidth = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalWidth += elementWidth(el);
               }
               int dx = (windowLength - totalWidth) / 2 + borderPadding;
               for (auto& el : elements)
               {
                  el->moveToPosition(Position(dx, borderPadding));
                  el->setPositions();
                  dx += elementWidth(el);
               }
               break;
            }
            case 1: // RIGHT MIDDLE
            {
               int totalHeight = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalHeight += elementHeight(el);
               }
               int dy = (windowHeight - totalHeight) / 2 + borderPadding;
               for (auto& el : elements)
               {
                  int x = windowLength - elementWidth(el) + borderPadding;
                  el->moveToPosition(Position(x, dy));
                  el->setPositions();
                  dy += elementHeight(el);
               }
               break;
            }
            case 2: // BOTTOM MIDDLE
            {
               int totalWidth = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalWidth += elementWidth(el);
               }
               int dx = (windowLength - totalWidth) / 2 + borderPadding;
               int y  = windowHeight - 1 + borderPadding;
               for (auto& el : elements)
               {
                  el->moveToPosition(Position(dx, y - elementHeight(el) + 1));
                  el->setPositions();
                  dx += elementWidth(el);
               }
               break;
            }
            case 3: // LEFT MIDDLE
            {
               int totalHeight = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalHeight += elementHeight(el);
               }
               int dy = (windowHeight - totalHeight) / 2 + borderPadding;
               for (auto& el : elements)
               {
                  el->moveToPosition(Position(borderPadding, dy));
                  el->setPositions();
                  dy += elementHeight(el);
               }
               break;
            }
            case 4: // CENTER
            {
               int totalWidth = 0;
               int maxHeight  = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalWidth += elementWidth(el);
                  if (elementHeight(el) > maxHeight)
                     maxHeight = elementHeight(el);
               }
               int dx = (windowLength - totalWidth) / 2 + borderPadding;
               int y  = (windowHeight - maxHeight) / 2 + borderPadding;
               for (auto& el : elements)
               {
                  el->moveToPosition(Position(dx, y));
                  el->setPositions();
                  dx += elementWidth(el);
               }
               break;
            }
            case 5: // TOP LEFT CORNER
            {
               int dx = borderPadding;
               int dy = borderPadding;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(dx, dy));
                  el->setPositions();

                  if (el->m_stackDirection == StackDirection::HORIZONTAL)
                  {
                     dx += elementWidth(el);
                  }
                  else
                  {
                     dy += elementHeight(el);
                  }
               }
               break;
            }
            case 6: // TOP RIGHT CORNER
            {
               int dx = windowLength + borderPadding;
               int dy = borderPadding;
               for (auto& el : elements)
               {
                  el->setPositions();

                  if (el->m_stackDirection == StackDirection::HORIZONTAL)
                  {
                     dx -= elementWidth(el);
                     el->moveToPosition(Position(dx, dy));
                  }
                  else
                  {
                     el->moveToPosition(Position(dx - elementWidth(el), dy));
                     dy += elementHeight(el);
                  }

                  el->setPositions();
               }
               break;
            }
            case 7: // BOTTOM LEFT CORNER
            {
               int dx = borderPadding;
               int dy = windowHeight + borderPadding;
               for (auto& el : elements)
               {
                  el->setPositions();

                  if (el->m_stackDirection == StackDirection::HORIZONTAL)
                  {
                     el->moveToPosition(Position(dx, dy - elementHeight(el)));
                     dx += elementWidth(el);
                  }
                  else
                  {
                     dy -= elementHeight(el);
                     el->moveToPosition(Position(dx, dy));
                  }

                  el->setPositions();
               }
               break;
            }
            case 8: // BOTTOM RIGHT CORNER
            {
               int dx = windowLength + borderPadding;
               int dy = windowHeight + borderPadding;
               for (auto& el : elements)
               {
                  el->setPositions();

                  if (el->m_stackDirection == StackDirection::HORIZONTAL)
                  {
                     dx -= elementWidth(el);
                     el->moveToPosition(Position(dx, dy - elementHeight(el)));
                  }
                  else
                  {
                     dy -= elementHeight(el);
                     el->moveToPosition(Position(dx - elementWidth(el), dy));
                  }

                  el->setPositions();
               }
               break;
            }
         }
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
void UIElement::updateAllLockedPositions()
{
   // Clean up expired elements first
   cleanupExpiredElements();

   // Update all windows by iterating through ncursesWindows
   std::set<WINDOW*> processedWindows;

   // Always process stdscr first
   updateWindowLockedPositions(stdscr);
   processedWindows.insert(stdscr);

   // Process all other windows
   for (auto& ncursesWindow : ncursesWindows)
   {
      WINDOW* window = ncursesWindow->getWindow();
      if (processedWindows.find(window) == processedWindows.end())
      {
         updateWindowLockedPositions(window);
         processedWindows.insert(window);
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
void UIElement::cleanupExpiredElements()
{
   // Helper function to remove elements that are no longer valid
   auto removeInactiveElements = [](std::vector<std::shared_ptr<UIElement>>& elements)
   {
      elements.erase(std::remove_if(elements.begin(), elements.end(),
                                    [](const std::shared_ptr<UIElement>& elem)
                                    {
                                       // Only remove if null - don't remove temporarily hidden elements
                                       return !elem;
                                    }),
                     elements.end());
   };

   // Clean up all positioning vectors
   removeInactiveElements(topMiddleUIElements);
   removeInactiveElements(rightMiddleUIElements);
   removeInactiveElements(bottomMiddleUIElements);
   removeInactiveElements(leftMiddleUIElements);
   removeInactiveElements(middleUIElements);
   removeInactiveElements(topLeftUIElements);
   removeInactiveElements(topRightUIElements);
   removeInactiveElements(bottomLeftUIElements);
   removeInactiveElements(bottomRightUIElements);
}

// public ----------------------------------------------------------------------------------------------------
void UIElement::removeFromPositioningVectors(std::shared_ptr<UIElement> element)
{
   if (!element)
      return;

   // Helper function to remove element from a vector
   auto removeFromVector = [&element](std::vector<std::shared_ptr<UIElement>>& vec)
   { vec.erase(std::remove(vec.begin(), vec.end(), element), vec.end()); };

   // Remove from all positioning vectors
   removeFromVector(topMiddleUIElements);
   removeFromVector(rightMiddleUIElements);
   removeFromVector(bottomMiddleUIElements);
   removeFromVector(leftMiddleUIElements);
   removeFromVector(middleUIElements);
   removeFromVector(topLeftUIElements);
   removeFromVector(topRightUIElements);
   removeFromVector(bottomLeftUIElements);
   removeFromVector(bottomRightUIElements);
}

// public ----------------------------------------------------------------------------------------------------
void UIElement::updateStdscrLockedPositions()
{
   // Helper lambdas
   auto elementWidth = [](const std::shared_ptr<UIElement>& el)
   { return el->m_maxPosition.getX() - el->m_minPosition.getX() + 1; };
   auto elementHeight = [](const std::shared_ptr<UIElement>& el)
   { return el->m_maxPosition.getY() - el->m_minPosition.getY() + 1; };

   // Helper function to find NcursesWindow from WINDOW*
   auto findNcursesWindow = [](WINDOW* window) -> std::shared_ptr<NcursesWindow>
   {
      for (auto& ncursesWindow : ncursesWindows)
      {
         if (ncursesWindow->getWindow() == window)
         {
            return ncursesWindow;
         }
      }
      return nullptr;
   };

   // Filter elements to only process those associated with stdscr
   auto filterStdscrElements = [](const std::vector<std::shared_ptr<UIElement>>& elements)
   {
      std::vector<std::shared_ptr<UIElement>> filtered;
      for (auto& el : elements)
      {
         WINDOW* window = el->getNcurseWindow();
         if (!window || window == stdscr)
         {
            filtered.push_back(el);
         }
      }
      return filtered;
   };

   // Filter each position group to only include stdscr elements
   auto filteredTopMiddle    = filterStdscrElements(topMiddleUIElements);
   auto filteredRightMiddle  = filterStdscrElements(rightMiddleUIElements);
   auto filteredBottomMiddle = filterStdscrElements(bottomMiddleUIElements);
   auto filteredLeftMiddle   = filterStdscrElements(leftMiddleUIElements);
   auto filteredMiddle       = filterStdscrElements(middleUIElements);
   auto filteredTopLeft      = filterStdscrElements(topLeftUIElements);
   auto filteredTopRight     = filterStdscrElements(topRightUIElements);
   auto filteredBottomLeft   = filterStdscrElements(bottomLeftUIElements);
   auto filteredBottomRight  = filterStdscrElements(bottomRightUIElements);

   // Group filtered elements by their window (should only be stdscr now)
   std::map<WINDOW*, std::vector<std::shared_ptr<UIElement>>> windowGroups[9];

   // Helper function to add elements to window groups
   auto addToWindowGroup = [&](const std::vector<std::shared_ptr<UIElement>>& elements, int groupIndex)
   {
      for (auto& el : elements)
      {
         WINDOW* window = el->getNcurseWindow();
         if (!window)
            window = stdscr;
         windowGroups[groupIndex][window].push_back(el);
      }
   };

   // Group all filtered elements by window and position
   addToWindowGroup(filteredTopMiddle, 0);
   addToWindowGroup(filteredRightMiddle, 1);
   addToWindowGroup(filteredBottomMiddle, 2);
   addToWindowGroup(filteredLeftMiddle, 3);
   addToWindowGroup(filteredMiddle, 4);
   addToWindowGroup(filteredTopLeft, 5);
   addToWindowGroup(filteredTopRight, 6);
   addToWindowGroup(filteredBottomLeft, 7);
   addToWindowGroup(filteredBottomRight, 8);

   // Process each window (should only be stdscr)
   for (int groupIndex = 0; groupIndex < 9; ++groupIndex)
   {
      for (auto& [window, elements] : windowGroups[groupIndex])
      {
         if (elements.empty())
            continue;

         // Get window dimensions and adjust for borders if enabled
         int windowHeight, windowLength;
         getmaxyx(window, windowHeight, windowLength);

         // Check if this window has borders and adjust usable area
         auto ncursesWindow = findNcursesWindow(window);
         int  borderPadding = 0;
         if (ncursesWindow && ncursesWindow->isBorderEnabled())
         {
            borderPadding = 1;
            windowHeight -= 2;
            windowLength -= 2;
         }

         // Process elements based on their position type (same logic as updateAllLockedPositions)
         switch (groupIndex)
         {
            case 0: // TOP MIDDLE
            {
               int totalWidth = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalWidth += elementWidth(el);
               }
               int dx = (windowLength - totalWidth) / 2 + borderPadding;
               for (auto& el : elements)
               {
                  el->moveToPosition(Position(dx, borderPadding));
                  el->setPositions();
                  dx += elementWidth(el);
               }
               break;
            }
            case 1: // RIGHT MIDDLE
            {
               int totalHeight = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalHeight += elementHeight(el);
               }
               int dy = (windowHeight - totalHeight) / 2 + borderPadding;
               for (auto& el : elements)
               {
                  int x = windowLength - elementWidth(el) + borderPadding;
                  el->moveToPosition(Position(x, dy));
                  el->setPositions();
                  dy += elementHeight(el);
               }
               break;
            }
            case 2: // BOTTOM MIDDLE
            {
               int totalWidth = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalWidth += elementWidth(el);
               }
               int dx = (windowLength - totalWidth) / 2 + borderPadding;
               int y  = windowHeight - 1 + borderPadding;
               for (auto& el : elements)
               {
                  el->moveToPosition(Position(dx, y - elementHeight(el) + 1));
                  el->setPositions();
                  dx += elementWidth(el);
               }
               break;
            }
            case 3: // LEFT MIDDLE
            {
               int totalHeight = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalHeight += elementHeight(el);
               }
               int dy = (windowHeight - totalHeight) / 2 + borderPadding;
               for (auto& el : elements)
               {
                  el->moveToPosition(Position(borderPadding, dy));
                  el->setPositions();
                  dy += elementHeight(el);
               }
               break;
            }
            case 4: // CENTER
            {
               int totalWidth = 0;
               int maxHeight  = 0;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(0, 0));
                  el->setPositions();
                  totalWidth += elementWidth(el);
                  if (elementHeight(el) > maxHeight)
                     maxHeight = elementHeight(el);
               }
               int dx = (windowLength - totalWidth) / 2 + borderPadding;
               int y  = (windowHeight - maxHeight) / 2 + borderPadding;
               for (auto& el : elements)
               {
                  el->moveToPosition(Position(dx, y));
                  el->setPositions();
                  dx += elementWidth(el);
               }
               break;
            }
            case 5: // TOP LEFT CORNER
            {
               int dx = borderPadding;
               int dy = borderPadding;
               for (auto& el : elements)
               {
                  el->setPositions();
                  el->moveToPosition(Position(dx, dy));
                  el->setPositions();

                  if (el->m_stackDirection == StackDirection::HORIZONTAL)
                  {
                     dx += elementWidth(el);
                  }
                  else
                  {
                     dy += elementHeight(el);
                  }
               }
               break;
            }
            case 6: // TOP RIGHT CORNER
            {
               int dx = windowLength + borderPadding;
               int dy = borderPadding;
               for (auto& el : elements)
               {
                  el->setPositions();

                  if (el->m_stackDirection == StackDirection::HORIZONTAL)
                  {
                     dx -= elementWidth(el);
                     el->moveToPosition(Position(dx, dy));
                  }
                  else
                  {
                     el->moveToPosition(Position(dx - elementWidth(el), dy));
                     dy += elementHeight(el);
                  }

                  el->setPositions();
               }
               break;
            }
            case 7: // BOTTOM LEFT CORNER
            {
               int dx = borderPadding;
               int dy = windowHeight + borderPadding;
               for (auto& el : elements)
               {
                  el->setPositions();

                  if (el->m_stackDirection == StackDirection::HORIZONTAL)
                  {
                     el->moveToPosition(Position(dx, dy - elementHeight(el)));
                     dx += elementWidth(el);
                  }
                  else
                  {
                     dy -= elementHeight(el);
                     el->moveToPosition(Position(dx, dy));
                  }

                  el->setPositions();
               }
               break;
            }
            case 8: // BOTTOM RIGHT CORNER
            {
               int dx = windowLength + borderPadding;
               int dy = windowHeight + borderPadding;
               for (auto& el : elements)
               {
                  el->setPositions();

                  if (el->m_stackDirection == StackDirection::HORIZONTAL)
                  {
                     dx -= elementWidth(el);
                     el->moveToPosition(Position(dx, dy - elementHeight(el)));
                  }
                  else
                  {
                     dy -= elementHeight(el);
                     el->moveToPosition(Position(dx - elementWidth(el), dy));
                  }

                  el->setPositions();
               }
               break;
            }
         }
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
void UIElement::displace(int dx, int dy)
{
   m_minPosition = Position(m_minPosition.getX() + dx, m_minPosition.getY() + dy);
   m_maxPosition = Position(m_maxPosition.getX() + dx, m_maxPosition.getY() + dy);
   for (Animation& animation : m_animations)
   {
      if (m_currentAnimationName == animation.getAnimationName())
      {
         m_dirtySprites.push_back(animation.getCurrentFrameSprite());
         animation.displace(dx, dy);
         break;
      }
   }
};

// public ----------------------------------------------------------------------------------------------------
void UIElement::setBorder(bool enabled)
{
   m_borderEnabled  = enabled;
   m_borderAutoSize = true;

   if (enabled)
   {
      calculateBorderDimensions();
      applyBorder();
   }
   else
   {
      removeBorder();
   }

   setPositions();
}

// public ----------------------------------------------------------------------------------------------------
void UIElement::setBorder(bool enabled, int width, int height)
{
   m_borderEnabled  = enabled;
   m_borderAutoSize = false;
   m_borderWidth    = width;
   m_borderHeight   = height;

   if (enabled)
   {
      applyBorder();
   }
   else
   {
      removeBorder();
   }

   setPositions();
}

// public ----------------------------------------------------------------------------------------------------
bool UIElement::isBorderEnabled() const
{
   return m_borderEnabled;
}

// protected -------------------------------------------------------------------------------------------------
void UIElement::applyBorder()
{
   if (!m_borderEnabled)
   {
      return;
   }

   // Get current animation and sprite
   Animation& animation = getCurrentAnimationMutable();
   Sprite&    sprite    = animation.getCurrentFrameSpriteMutable();

   // Use the original pixels as the base (without any borders)
   std::vector<Pixel> originalPixels = m_originalPixels;

   // Calculate border dimensions if auto-sizing
   if (m_borderAutoSize)
   {
      calculateBorderDimensions();
   }

   // Find the bounds of original content
   int  minX = 0, maxX = 0, minY = 0, maxY = 0;
   bool firstPixel = true;

   for (const Pixel& pixel : originalPixels)
   {
      int x = pixel.getPosition().getX();
      int y = pixel.getPosition().getY();

      if (firstPixel)
      {
         minX = maxX = x;
         minY = maxY = y;
         firstPixel  = false;
      }
      else
      {
         if (x < minX)
            minX = x;
         if (x > maxX)
            maxX = x;
         if (y < minY)
            minY = y;
         if (y > maxY)
            maxY = y;
      }
   }

   // Shift existing content to make room for border
   std::vector<Pixel> newPixels;
   for (const Pixel& pixel : originalPixels)
   {
      Position newPos(pixel.getPosition().getX() + 1, pixel.getPosition().getY() + 1);
      Pixel    newPixel(newPos, pixel.getCharacter(), pixel.getTextColor(), pixel.getBackgroundColor(),
                        pixel.getAttributes());
      newPixels.push_back(newPixel);
   }

   // Border characters
   wchar_t topLeft = L'+', topRight = L'+', bottomLeft = L'+', bottomRight = L'+';
   wchar_t horizontal = L'-', vertical = L'|';

   // Add border pixels
   int borderWidth  = m_borderAutoSize ? (maxX - minX + 3) : m_borderWidth;
   int borderHeight = m_borderAutoSize ? (maxY - minY + 3) : m_borderHeight;

   // Top border
   for (int x = 0; x < borderWidth; x++)
   {
      wchar_t ch = (x == 0) ? topLeft : ((x == borderWidth - 1) ? topRight : horizontal);
      newPixels.push_back(Pixel(Position(x, 0), ch));
   }

   // Bottom border
   for (int x = 0; x < borderWidth; x++)
   {
      wchar_t ch = (x == 0) ? bottomLeft : ((x == borderWidth - 1) ? bottomRight : horizontal);
      newPixels.push_back(Pixel(Position(x, borderHeight - 1), ch));
   }

   // Left and right borders
   for (int y = 1; y < borderHeight - 1; y++)
   {
      newPixels.push_back(Pixel(Position(0, y), vertical));
      newPixels.push_back(Pixel(Position(borderWidth - 1, y), vertical));
   }

   // Update sprite with new pixels
   sprite.setPixels(newPixels);
   sprite.setAnchor(Position(0, 0));
}

// protected -------------------------------------------------------------------------------------------------
void UIElement::removeBorder()
{
   // Get current animation and sprite
   Animation& animation = getCurrentAnimationMutable();
   Sprite&    sprite    = animation.getCurrentFrameSpriteMutable();

   // Restore the original sprite (without borders)
   sprite.setPixels(m_originalPixels);
   sprite.setAnchor(Position(0, 0));
}

// protected -------------------------------------------------------------------------------------------------
void UIElement::calculateBorderDimensions()
{
   // Get current animation and sprite
   const Animation&          animation     = getCurrentAnimation();
   const Sprite&             sprite        = animation.getCurrentFrameSprite();
   const std::vector<Pixel>& currentPixels = sprite.getPixels();

   if (currentPixels.empty())
   {
      m_borderWidth  = 4; // Minimum border size
      m_borderHeight = 3;
      return;
   }

   // Find the bounds of current content
   int  minX = 0, maxX = 0, minY = 0, maxY = 0;
   bool firstPixel = true;

   for (const Pixel& pixel : currentPixels)
   {
      int x = pixel.getPosition().getX();
      int y = pixel.getPosition().getY();

      if (firstPixel)
      {
         minX = maxX = x;
         minY = maxY = y;
         firstPixel  = false;
      }
      else
      {
         if (x < minX)
            minX = x;
         if (x > maxX)
            maxX = x;
         if (y < minY)
            minY = y;
         if (y > maxY)
            maxY = y;
      }
   }

   // Calculate border dimensions (content + 2 for border on each side)
   m_borderWidth  = (maxX - minX) + 3;
   m_borderHeight = (maxY - minY) + 3;
}

// protected -------------------------------------------------------------------------------------------------
void UIElement::storeOriginalSprite()
{
   // This method is now handled by Button::setText() for buttons
   // For other UI elements, just store the current pixels as-is
   const Animation& animation = getCurrentAnimation();
   const Sprite&    sprite    = animation.getCurrentFrameSprite();
   m_originalPixels           = sprite.getPixels();
}