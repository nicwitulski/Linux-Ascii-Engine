//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file NcursesWindow.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of the NcursesWindow class
/// @version 0.1
/// @date 2025-07-08
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../include/NcursesWindow.h"
#include "../../include/ColorManager.h"
#include "../../include/Parameters.h"
#include "../../include/UIElement.h"
#include <algorithm>
#include <climits>

// public ----------------------------------------------------------------------------------------------------
NcursesWindow::~NcursesWindow()
{
   // Clean up sub-windows first
   for (auto& subWindow : m_subWindows)
   {
      if (subWindow && subWindow->m_window)
      {
         delwin(subWindow->m_window);
         subWindow->m_window = nullptr;
      }
   }
   m_subWindows.clear();

   // Clean up our own window (but only if it's not a sub-window,
   // as sub-windows are cleaned up by their parent or by removeSubWindow)
   if (!m_isSubWindow && m_window && m_window != stdscr)
   {
      delwin(m_window);
   }
}

// public ----------------------------------------------------------------------------------------------------
NcursesWindow::NcursesWindow(int length, int height, int windowLayer, bool isMoveableByCamera, int posX,
                             int posY)
{
   m_currentHeight        = height;
   m_currentLength        = length;
   m_originalHeight       = height;
   m_originalLength       = length;
   m_window               = newwin(height, length, posY, posX);
   m_windowLayer          = windowLayer;
   m_displayNeedsCleared  = true;
   m_printablesNeedSorted = true;
   m_isMoveableByCamera   = isMoveableByCamera;
   m_basePositionX        = posX;
   m_basePositionY        = posY;
   m_drawBorder           = true;
   m_isDraggable          = true;
   m_autoResize           = false;
   m_paddingX             = 1;
   m_paddingY             = 1;
   m_minWidth             = 3;
   m_minHeight            = 3;
   m_isSubWindow          = false;

   clearBuffer();
};

// public ----------------------------------------------------------------------------------------------------
NcursesWindow::NcursesWindow(WINDOW* window, int windowLayer, bool isMoveableByCamera)
{
   m_window               = window;
   m_windowLayer          = windowLayer;
   m_displayNeedsCleared  = true;
   m_printablesNeedSorted = true;
   m_isMoveableByCamera   = isMoveableByCamera;
   m_basePositionX        = 0;
   m_basePositionY        = 0;
   m_drawBorder           = true;
   m_isDraggable          = true;
   m_autoResize           = false;
   m_paddingX             = 1;
   m_paddingY             = 1;
   m_minWidth             = 3;
   m_minHeight            = 3;
   m_isSubWindow          = false;

   getmaxyx(m_window, m_currentHeight, m_currentLength);
   m_originalHeight = m_currentHeight;
   m_originalLength = m_currentLength;
   clearBuffer();
}

// public ----------------------------------------------------------------------------------------------------
NcursesWindow::NcursesWindow(bool autoResize, int windowLayer, bool isMoveableByCamera, int posX, int posY,
                             int paddingX, int paddingY, int minWidth, int minHeight)
{
   // Start with minimum dimensions - will be resized automatically when content is added
   m_currentHeight        = std::max(1, minHeight);
   m_currentLength        = std::max(1, minWidth);
   m_originalHeight       = m_currentHeight;
   m_originalLength       = m_currentLength;
   m_window               = newwin(m_currentHeight, m_currentLength, posY, posX);
   m_windowLayer          = windowLayer;
   m_displayNeedsCleared  = true;
   m_printablesNeedSorted = true;
   m_isMoveableByCamera   = isMoveableByCamera;
   m_basePositionX        = posX;
   m_basePositionY        = posY;
   m_drawBorder           = true;
   m_isDraggable          = true;
   m_autoResize           = autoResize;
   m_paddingX             = std::max(0, paddingX);
   m_paddingY             = std::max(0, paddingY);
   m_minWidth             = std::max(1, minWidth);
   m_minHeight            = std::max(1, minHeight);
   m_isSubWindow          = false;

   clearBuffer();
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::addPrintable(std::shared_ptr<Printable> printable)
{
   m_containedPrintables.push_back(printable);

   // Trigger auto-resize if enabled
   if (m_autoResize)
   {
      resizeToFitContents();
   }
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::removePrintable(std::shared_ptr<Printable> printable)
{
   m_containedPrintables.erase(std::remove(m_containedPrintables.begin(), m_containedPrintables.end(),
                                           printable),
                               m_containedPrintables.end());

   // Trigger auto-resize if enabled
   if (m_autoResize)
   {
      resizeToFitContents();
   }
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::clearPrintables()
{
   m_containedPrintables.clear();

   // Trigger auto-resize if enabled
   if (m_autoResize)
   {
      resizeToFitContents();
   }
}

// public ----------------------------------------------------------------------------------------------------
const int& NcursesWindow::getWindowLayer() const
{
   return m_windowLayer;
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::setWindowLayer(int windowLayer)
{
   m_windowLayer = windowLayer;
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::clearBuffer()
{
   m_currentFrameBuffer =
         std::vector<std::vector<Pixel>>(m_currentHeight, std::vector<Pixel>(m_currentLength, Pixel()));
   m_lastFrameBuffer = m_currentFrameBuffer;
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::closeWindow()
{
   delwin(m_window);
}

// public ----------------------------------------------------------------------------------------------------
WINDOW* NcursesWindow::getWindow()
{
   return m_window;
}

// public ----------------------------------------------------------------------------------------------------
const bool& NcursesWindow::getDisplayNeedsCleared() const
{
   return m_displayNeedsCleared;
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::setDisplayNeedsCleared(const bool displayNeedsCleared)
{
   m_displayNeedsCleared = displayNeedsCleared;
}

// public ----------------------------------------------------------------------------------------------------
const bool& NcursesWindow::getPrintablesNeedSorted() const
{
   return m_printablesNeedSorted;
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::setPrintablesNeedSorted(const bool printablesNeedSorted)
{
   m_printablesNeedSorted = printablesNeedSorted;
}

// public ----------------------------------------------------------------------------------------------------
bool NcursesWindow::isMoveableByCamera() const
{
   return m_isMoveableByCamera;
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::setMoveableByCamera(const bool moveable)
{
   m_isMoveableByCamera = moveable;
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::updateWindowPosition()
{
   if (m_isMoveableByCamera && currentCamera)
   {
      int newX = m_basePositionX + currentCamera->getLengthOffset();
      int newY = m_basePositionY + currentCamera->getHeightOffset();

      // Check if window is actually moving to avoid unnecessary clearing
      int currentX, currentY;
      getbegyx(m_window, currentY, currentX);
      if (currentX != newX || currentY != newY)
      {
         displayNeedsCleared = true;
      }

      mvwin(m_window, newY, newX);

      // Update all sub-windows to maintain their relative positions
      for (auto& subWindow : m_subWindows)
      {
         if (subWindow && subWindow->m_window)
         {
            mvderwin(subWindow->m_window, subWindow->m_basePositionY, subWindow->m_basePositionX);
         }
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::setBasePosition(const int x, const int y)
{
   // Only clear display if the window is actually moving to a different position
   if (m_basePositionX != x || m_basePositionY != y)
   {
      displayNeedsCleared = true;
   }

   m_basePositionX = x;
   m_basePositionY = y;
   if (!m_isMoveableByCamera)
   {
      mvwin(m_window, y, x);

      // Update all sub-windows to maintain their relative positions
      for (auto& subWindow : m_subWindows)
      {
         if (subWindow && subWindow->m_window)
         {
            mvderwin(subWindow->m_window, subWindow->m_basePositionY, subWindow->m_basePositionX);
         }
      }
   }
   else
   {
      updateWindowPosition();
   }
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::printPixel(const Pixel pixel, const bool isMoveableByCamera)
{
   int printedX = pixel.getPosition().getX();
   int printedY = pixel.getPosition().getY();

   if (isMoveableByCamera && currentCamera)
   {
      printedX += currentCamera->getLengthOffset();
      printedY += currentCamera->getHeightOffset();
   }

   if (printedX >= 0 && printedX < m_currentLength && printedY >= 0 && printedY < m_currentHeight)
   {
      m_currentFrameBuffer[printedY][printedX] = pixel;
   }
}

// public ---------------------------------------------------------------------------------------------
void NcursesWindow::printSprite(const Sprite sprite, const bool isMoveableByCamera)
{
   for (const Pixel& pixel : sprite.getPixels())
   {
      printPixel(pixel, isMoveableByCamera);
   }
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::eraseSprite(const Sprite sprite, const bool isMoveableByCamera)
{
   for (const Pixel& pixel : sprite.getPixels())
   {
      printPixel(Pixel(pixel.getPosition(), ' '), isMoveableByCamera);
   }
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::refreshWindow(const float deltaTime)
{
   // Update window position based on camera if moveable
   updateWindowPosition();

   // Handle window resizing
   if (m_window == stdscr)
   {
      // Only stdscr should resize with the terminal
      int tempHeight, tempLength;
      getmaxyx(m_window, tempHeight, tempLength);

      if (tempHeight != m_currentHeight || tempLength != m_currentLength)
      {
         m_currentHeight  = tempHeight;
         m_currentLength  = tempLength;
         m_originalHeight = tempHeight;
         m_originalLength = tempLength;

         // Update global screen dimensions
         SCREEN_HEIGHT = tempHeight;
         SCREEN_LENGTH = tempLength;

         UIElement::updateStdscrLockedPositions();

         m_currentFrameBuffer =
               std::vector<std::vector<Pixel>>(m_currentHeight, std::vector<Pixel>(m_currentLength, Pixel()));
         m_lastFrameBuffer     = m_currentFrameBuffer;
         m_displayNeedsCleared = true;
      }
   }
   else
   {
      // Fixed-size windows should maintain their original dimensions
      // If dimensions changed (shouldn't happen), recreate the window
      int tempHeight, tempLength;
      getmaxyx(m_window, tempHeight, tempLength);

      if (tempHeight != m_originalHeight || tempLength != m_originalLength)
      {
         // Store sub-window information before recreation
         struct SubWindowInfo
         {
            int                                     width, height, relativeX, relativeY, layer;
            bool                                    borderEnabled;
            std::vector<std::shared_ptr<Printable>> printables;
            std::shared_ptr<NcursesWindow>          originalSubWindow;
         };
         std::vector<SubWindowInfo> subWindowInfos;

         for (auto& subWindow : m_subWindows)
         {
            if (subWindow)
            {
               SubWindowInfo info;
               info.width             = subWindow->m_currentLength;
               info.height            = subWindow->m_currentHeight;
               info.relativeX         = subWindow->m_basePositionX;
               info.relativeY         = subWindow->m_basePositionY;
               info.layer             = subWindow->m_windowLayer;
               info.borderEnabled     = subWindow->m_drawBorder;
               info.printables        = subWindow->m_containedPrintables;
               info.originalSubWindow = subWindow;
               subWindowInfos.push_back(info);
            }
         }

         // Recreate window with original dimensions
         delwin(m_window);
         m_window        = newwin(m_originalHeight, m_originalLength, m_basePositionY, m_basePositionX);
         m_currentHeight = m_originalHeight;
         m_currentLength = m_originalLength;

         m_currentFrameBuffer =
               std::vector<std::vector<Pixel>>(m_currentHeight, std::vector<Pixel>(m_currentLength, Pixel()));
         m_lastFrameBuffer     = m_currentFrameBuffer;
         m_displayNeedsCleared = true;

         // Clear the old sub-windows list
         m_subWindows.clear();

         // Recreate sub-windows
         for (const auto& info : subWindowInfos)
         {
            // Recreate sub-window regardless of bounds (allow natural cut-off)
            auto newSubWindow =
                  createSubWindow(info.width, info.height, info.relativeX, info.relativeY, info.layer);
            if (newSubWindow)
            {
               newSubWindow->setBorderEnabled(info.borderEnabled);

               // Restore printables and update their window association
               for (auto& printable : info.printables)
               {
                  newSubWindow->addPrintable(printable);
                  printable->setNcurseWindow(newSubWindow->getWindow());
               }
            }
         }

         // Update UI elements associated with this window after recreation
         UIElement::updateWindowLockedPositions(m_window);
      }
   }

   // Check global display clear flag (set by camera movement)
   if (m_displayNeedsCleared || displayNeedsCleared)
   {
      werase(m_window);
      for (auto& row : m_currentFrameBuffer)
         std::fill(row.begin(), row.end(), Pixel());
      for (auto& row : m_lastFrameBuffer)
         std::fill(row.begin(), row.end(), Pixel());
      m_displayNeedsCleared = false;
   }

   refreshPrintables(deltaTime);

   // Draw diffs
   for (int y = 0; y < m_currentHeight; ++y)
   {
      for (int x = 0; x < m_currentLength; ++x)
      {
         const Pixel& curr = m_currentFrameBuffer[y][x];
         const Pixel& last = m_lastFrameBuffer[y][x];
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

            wattrset(m_window, attr);
            if (has_colors())
            {
               wcolor_set(m_window, colorPair, NULL);
            }

            // Use wide-character function for Unicode support
            cchar_t wch;
            wch.chars[0]  = curr.getCharacter();
            wch.chars[1]  = L'\0';
            wch.attr      = attr;
            wch.ext_color = colorPair;
            mvwadd_wch(m_window, y, x, &wch);

            wattroff(m_window, attr);
            if (has_colors())
            {
               wcolor_set(m_window, 0, NULL); // Reset to default after drawing
            }

            m_lastFrameBuffer[y][x] = curr;
         }
      }
   }

   // Draw border if enabled
   if (m_drawBorder)
   {
      box(m_window, 0, 0);
   }

   wnoutrefresh(m_window);
   curs_set(0);
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::refreshPrintables(const float deltaTime)
{
   if (m_printablesNeedSorted)
   {
      std::sort(m_containedPrintables.begin(), m_containedPrintables.end(),
                [](const std::shared_ptr<Printable>& a, const std::shared_ptr<Printable>& b)
                {
                   return a->getCurrentAnimation().getCurrentFrameSprite().getLayer() <
                          b->getCurrentAnimation().getCurrentFrameSprite().getLayer();
                });
      m_printablesNeedSorted = false;
   }

   for (auto& printable : m_containedPrintables)
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

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::setBorderEnabled(const bool enabled)
{
   m_drawBorder = enabled;
}

// public ----------------------------------------------------------------------------------------------------
bool NcursesWindow::isBorderEnabled() const
{
   return m_drawBorder;
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::setDraggable(const bool draggable)
{
   m_isDraggable = draggable;
}

// public ----------------------------------------------------------------------------------------------------
bool NcursesWindow::isDraggable() const
{
   return m_isDraggable;
}

// public ----------------------------------------------------------------------------------------------------
bool NcursesWindow::isMouseOnBorder(const Position& mousePos) const
{
   if (!m_drawBorder || !isMouseInWindow(mousePos))
   {
      return false;
   }

   int windowX, windowY;
   getbegyx(m_window, windowY, windowX);

   int relativeX = mousePos.getX() - windowX;
   int relativeY = mousePos.getY() - windowY;

   // Check if mouse is on border (edges of the window)
   return (relativeX == 0 || relativeX == m_currentLength - 1 || relativeY == 0 ||
           relativeY == m_currentHeight - 1);
}

// public ----------------------------------------------------------------------------------------------------
bool NcursesWindow::isMouseInWindow(const Position& mousePos) const
{
   int windowX, windowY;
   getbegyx(m_window, windowY, windowX);

   int mouseX = mousePos.getX();
   int mouseY = mousePos.getY();

   return (mouseX >= windowX && mouseX < windowX + m_currentLength && mouseY >= windowY &&
           mouseY < windowY + m_currentHeight);
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::setAutoResize(const bool autoResize)
{
   m_autoResize = autoResize;

   // If auto-resize is enabled, immediately resize to fit current contents
   if (m_autoResize)
   {
      resizeToFitContents();
   }
}

// public ----------------------------------------------------------------------------------------------------
bool NcursesWindow::isAutoResize() const
{
   return m_autoResize;
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::setPadding(const int paddingX, const int paddingY)
{
   m_paddingX = std::max(0, paddingX);
   m_paddingY = std::max(0, paddingY);

   // Trigger resize if auto-resize is enabled
   if (m_autoResize)
   {
      resizeToFitContents();
   }
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::setMinimumSize(const int minWidth, const int minHeight)
{
   m_minWidth  = std::max(1, minWidth);
   m_minHeight = std::max(1, minHeight);

   // Trigger resize if auto-resize is enabled
   if (m_autoResize)
   {
      resizeToFitContents();
   }
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::resizeToFitContents()
{
   // Don't resize stdscr or sub-windows
   if (m_window == stdscr || m_isSubWindow)
   {
      return;
   }

   int requiredWidth, requiredHeight;
   if (calculateRequiredSize(requiredWidth, requiredHeight))
   {
      performResize(requiredWidth, requiredHeight);
   }
}

// private ---------------------------------------------------------------------------------------------------
bool NcursesWindow::calculateContentBounds(int& minX, int& minY, int& maxX, int& maxY)
{
   if (m_containedPrintables.empty())
   {
      return false;
   }

   minX = INT_MAX;
   minY = INT_MAX;
   maxX = INT_MIN;
   maxY = INT_MIN;

   bool hasContent = false;

   for (const auto& printable : m_containedPrintables)
   {
      if (!printable->isVisable())
      {
         continue;
      }

      const Animation&          currentAnim   = printable->getCurrentAnimation();
      const Sprite&             currentSprite = currentAnim.getCurrentFrameSprite();
      const std::vector<Pixel>& pixels        = currentSprite.getPixels();

      for (const Pixel& pixel : pixels)
      {
         int x = pixel.getPosition().getX();
         int y = pixel.getPosition().getY();

         minX       = std::min(minX, x);
         minY       = std::min(minY, y);
         maxX       = std::max(maxX, x);
         maxY       = std::max(maxY, y);
         hasContent = true;
      }
   }

   return hasContent;
}

// private ---------------------------------------------------------------------------------------------------
bool NcursesWindow::calculateRequiredSize(int& requiredWidth, int& requiredHeight)
{
   int minX, minY, maxX, maxY;

   if (!calculateContentBounds(minX, minY, maxX, maxY))
   {
      // No content, use minimum size
      requiredWidth  = m_minWidth;
      requiredHeight = m_minHeight;
   }
   else
   {
      // Calculate required size based on content bounds
      int contentWidth  = maxX - minX + 1;
      int contentHeight = maxY - minY + 1;

      requiredWidth  = contentWidth + (2 * m_paddingX);
      requiredHeight = contentHeight + (2 * m_paddingY);

      // Account for border if enabled
      if (m_drawBorder)
      {
         requiredWidth += 2;
         requiredHeight += 2;
      }
   }

   // Ensure minimum size
   requiredWidth  = std::max(requiredWidth, m_minWidth);
   requiredHeight = std::max(requiredHeight, m_minHeight);

   // Check if resize is needed
   return (requiredWidth != m_currentLength || requiredHeight != m_currentHeight);
}

// private ---------------------------------------------------------------------------------------------------
void NcursesWindow::performResize(int newWidth, int newHeight)
{
   // Store current sub-window information
   struct SubWindowInfo
   {
      int                                     width, height, relativeX, relativeY, layer;
      bool                                    borderEnabled;
      std::vector<std::shared_ptr<Printable>> printables;
      std::shared_ptr<NcursesWindow>          originalSubWindow;
   };
   std::vector<SubWindowInfo> subWindowInfos;

   for (auto& subWindow : m_subWindows)
   {
      if (subWindow)
      {
         SubWindowInfo info;
         info.width             = subWindow->m_currentLength;
         info.height            = subWindow->m_currentHeight;
         info.relativeX         = subWindow->m_basePositionX;
         info.relativeY         = subWindow->m_basePositionY;
         info.layer             = subWindow->m_windowLayer;
         info.borderEnabled     = subWindow->m_drawBorder;
         info.printables        = subWindow->m_containedPrintables;
         info.originalSubWindow = subWindow;
         subWindowInfos.push_back(info);
      }
   }

   // Recreate window with new dimensions
   delwin(m_window);
   m_window         = newwin(newHeight, newWidth, m_basePositionY, m_basePositionX);
   m_currentHeight  = newHeight;
   m_currentLength  = newWidth;
   m_originalHeight = newHeight;
   m_originalLength = newWidth;

   // Clear buffers and update them for new size
   m_currentFrameBuffer =
         std::vector<std::vector<Pixel>>(m_currentHeight, std::vector<Pixel>(m_currentLength, Pixel()));
   m_lastFrameBuffer     = m_currentFrameBuffer;
   m_displayNeedsCleared = true;

   // Clear the old sub-windows list
   m_subWindows.clear();

   // Recreate sub-windows
   for (const auto& info : subWindowInfos)
   {
      // Only recreate if sub-window fits within new bounds
      if (info.relativeX + info.width <= newWidth && info.relativeY + info.height <= newHeight)
      {
         auto newSubWindow =
               createSubWindow(info.width, info.height, info.relativeX, info.relativeY, info.layer);
         if (newSubWindow)
         {
            newSubWindow->setBorderEnabled(info.borderEnabled);

            // Restore printables and update their window association
            for (auto& printable : info.printables)
            {
               newSubWindow->addPrintable(printable);
               printable->setNcurseWindow(newSubWindow->getWindow());
            }
         }
      }
   }

   // Update UI elements associated with this window after resize
   UIElement::updateWindowLockedPositions(m_window);
}

// public ----------------------------------------------------------------------------------------------------
std::shared_ptr<NcursesWindow> NcursesWindow::createSubWindow(int width, int height, int relativeX,
                                                              int relativeY, int windowLayer)
{
   // Create derived window (positioned relative to parent)
   WINDOW* subWin = derwin(m_window, height, width, relativeY, relativeX);
   if (!subWin)
   {
      return nullptr; // Failed to create sub-window
   }

   // Create NcursesWindow wrapper for the sub-window using existing constructor
   auto subWindow = std::shared_ptr<NcursesWindow>(new NcursesWindow(subWin, windowLayer, false));

   // Override specific settings for sub-windows
   subWindow->m_basePositionX = relativeX;
   subWindow->m_basePositionY = relativeY;
   subWindow->m_isSubWindow   = true;
   subWindow->m_parentWindow  = shared_from_this();

   // Add to our sub-windows list
   m_subWindows.push_back(subWindow);

   return subWindow;
}

// public ----------------------------------------------------------------------------------------------------
void NcursesWindow::removeSubWindow(std::shared_ptr<NcursesWindow> subWindow)
{
   if (!subWindow)
      return;

   // Remove from sub-windows list
   m_subWindows.erase(std::remove(m_subWindows.begin(), m_subWindows.end(), subWindow), m_subWindows.end());

   // Clean up the sub-window
   if (subWindow->m_window)
   {
      delwin(subWindow->m_window);
      subWindow->m_window = nullptr;
   }
}

// public ----------------------------------------------------------------------------------------------------
const std::vector<std::shared_ptr<NcursesWindow>>& NcursesWindow::getSubWindows() const
{
   return m_subWindows;
}

// public ----------------------------------------------------------------------------------------------------
std::shared_ptr<NcursesWindow> NcursesWindow::getParentWindow() const
{
   return m_parentWindow.lock();
}

// public ----------------------------------------------------------------------------------------------------
bool NcursesWindow::isSubWindow() const
{
   return m_isSubWindow;
}