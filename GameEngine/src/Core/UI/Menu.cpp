//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Menu.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of scrollable menu system for directory browsing and item selection
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/Menu.h"
#include "../../../include/Frame.h"
#include "../../../include/RGB.h"
#include "../../../include/Sprite.h"
#include <ncurses.h>
#include <algorithm>

// public ----------------------------------------------------------------------------------------------------
Menu::Menu(const std::vector<std::string>& menuItems, int maxVisibleItems, int width)
   : UIElement(),
     m_menuItems(menuItems),
     m_selectedIndex(0),
     m_scrollOffset(0),
     m_maxVisibleItems(maxVisibleItems),
     m_menuWidth(width),
     m_isActive(false)
{
   m_menuHeight = std::min((int)m_menuItems.size() + 2, m_maxVisibleItems + 2); // +2 for borders

   // Generate the menu sprite first
   generateMenuSprite();

   // Set positions after generating sprite (needed for dynamic positioning)
   setPositions();

   setVisability(true);
   setMoveableByCamera(false);
}

// public ----------------------------------------------------------------------------------------------------
void Menu::setOnSelectCallback(std::function<void(const std::string&)> callback)
{
   m_onSelectCallback = callback;
}

// public ----------------------------------------------------------------------------------------------------
void Menu::setOnCancelCallback(std::function<void()> callback)
{
   m_onCancelCallback = callback;
}

// public ----------------------------------------------------------------------------------------------------
bool Menu::handleInput(int key)
{
   if (!m_isActive || m_menuItems.empty())
   {
      return false;
   }

   switch (key)
   {
      case KEY_UP:
         moveUp();
         return true;
      case KEY_DOWN:
         moveDown();
         return true;
      case '\n':
      case '\r':
      case KEY_ENTER:
         selectCurrent();
         return true;
      case 27: // ESC key
         cancel();
         return true;
      default:
         return false;
   }
}

// public ----------------------------------------------------------------------------------------------------
void Menu::moveUp()
{
   if (m_menuItems.empty())
      return;

   m_selectedIndex = (m_selectedIndex - 1 + m_menuItems.size()) % m_menuItems.size();
   updateScrollOffset();
   generateMenuSprite();
   // Don't call setPositions() - menu bounds haven't changed, only selection highlighting
}

// public ----------------------------------------------------------------------------------------------------
void Menu::moveDown()
{
   if (m_menuItems.empty())
      return;

   m_selectedIndex = (m_selectedIndex + 1) % m_menuItems.size();
   updateScrollOffset();
   generateMenuSprite();
   // Don't call setPositions() - menu bounds haven't changed, only selection highlighting
}

// public ----------------------------------------------------------------------------------------------------
void Menu::selectCurrent()
{
   if (!m_menuItems.empty() && m_onSelectCallback)
   {
      m_onSelectCallback(m_menuItems[m_selectedIndex]);
   }
}

// public ----------------------------------------------------------------------------------------------------
void Menu::cancel()
{
   if (m_onCancelCallback)
   {
      m_onCancelCallback();
   }
}

// public ----------------------------------------------------------------------------------------------------
void Menu::setActive(bool active)
{
   m_isActive = active;
   if (active)
   {
      generateMenuSprite();
      // Don't call setPositions() - menu bounds haven't changed, only visibility
   }
}

// public ----------------------------------------------------------------------------------------------------
bool Menu::isActive() const
{
   return m_isActive;
}

// public ----------------------------------------------------------------------------------------------------
void Menu::setMenuItems(const std::vector<std::string>& items)
{
   m_menuItems     = items;
   m_selectedIndex = 0;
   m_scrollOffset  = 0;
   m_menuHeight    = std::min((int)m_menuItems.size() + 2, m_maxVisibleItems + 2);
   generateMenuSprite();
   setPositions(); // Keep this one - menu size may have actually changed
}

// public ----------------------------------------------------------------------------------------------------
std::string Menu::getSelectedItem() const
{
   if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_menuItems.size())
   {
      return m_menuItems[m_selectedIndex];
   }
   return "";
}

// public ----------------------------------------------------------------------------------------------------
int Menu::getSelectedIndex() const
{
   return m_selectedIndex;
}

// private ---------------------------------------------------------------------------------------------------
void Menu::updateScrollOffset()
{
   if (m_menuItems.empty())
      return;

   // Ensure selected item is visible
   if (m_selectedIndex < m_scrollOffset)
   {
      m_scrollOffset = m_selectedIndex;
   }
   else if (m_selectedIndex >= m_scrollOffset + m_maxVisibleItems)
   {
      m_scrollOffset = m_selectedIndex - m_maxVisibleItems + 1;
   }

   // Ensure scroll offset doesn't go negative
   m_scrollOffset = std::max(0, m_scrollOffset);

   // Ensure we don't scroll past the end
   int maxScroll  = std::max(0, (int)m_menuItems.size() - m_maxVisibleItems);
   m_scrollOffset = std::min(m_scrollOffset, maxScroll);
}

// private ---------------------------------------------------------------------------------------------------
void Menu::generateMenuSprite()
{
   // Store current anchor position to preserve positioning
   Position currentAnchor(0, 0);
   if (!m_animations.empty())
   {
      currentAnchor = getCurrentAnimation().getCurrentFrameSprite().getAnchor();
   }

   std::vector<Pixel> pixels;

   // Define colors
   RGB normalTextColor(1000, 1000, 1000);   // White
   RGB selectedTextColor(1000, 1000, 1000); // White text
   RGB normalBgColor(0, 0, 0);              // Black background
   RGB selectedBgColor(500, 500, 500);      // Gray background for selection
   RGB borderColor(800, 800, 800);          // Light gray

   // Draw top border
   for (int x = 0; x < m_menuWidth; ++x)
   {
      wchar_t ch = (x == 0) ? L'┌' : (x == m_menuWidth - 1) ? L'┐' : L'─';
      pixels.push_back(Pixel(Position(x, 0), ch, borderColor, normalBgColor, A_NORMAL));
   }

   // Draw menu items
   int visibleItemCount = std::min(m_maxVisibleItems, (int)m_menuItems.size());
   for (int i = 0; i < visibleItemCount; ++i)
   {
      int itemIndex = m_scrollOffset + i;
      int y         = i + 1; // +1 for top border

      // Draw left border
      pixels.push_back(Pixel(Position(0, y), L'│', borderColor, normalBgColor, A_NORMAL));

      // Draw item text
      std::string itemText = m_menuItems[itemIndex];

      // Truncate text if too long
      int availableWidth = m_menuWidth - 4; // -4 for borders and padding
      if ((int)itemText.length() > availableWidth)
      {
         itemText = itemText.substr(0, availableWidth - 3) + "...";
      }

      // Determine colors based on selection
      RGB    textColor = (itemIndex == m_selectedIndex) ? selectedTextColor : normalTextColor;
      RGB    bgColor   = (itemIndex == m_selectedIndex) ? selectedBgColor : normalBgColor;
      attr_t attr      = (itemIndex == m_selectedIndex) ? A_BOLD : A_NORMAL;

      // Add selection indicator or padding space
      wchar_t indicator = (itemIndex == m_selectedIndex) ? L'>' : L' ';
      pixels.push_back(Pixel(Position(1, y), indicator, textColor, bgColor, attr));

      // Add item text
      for (int x = 0; x < (int)itemText.length() && x < availableWidth; ++x)
      {
         pixels.push_back(Pixel(Position(x + 2, y), itemText[x], textColor, bgColor, attr));
      }

      // Fill remaining space
      for (int x = itemText.length() + 2; x < m_menuWidth - 1; ++x)
      {
         pixels.push_back(Pixel(Position(x, y), L' ', textColor, bgColor, attr));
      }

      // Draw right border
      pixels.push_back(Pixel(Position(m_menuWidth - 1, y), L'│', borderColor, normalBgColor, A_NORMAL));
   }

   // Draw bottom border
   int bottomY = visibleItemCount + 1;
   for (int x = 0; x < m_menuWidth; ++x)
   {
      wchar_t ch = (x == 0) ? L'└' : (x == m_menuWidth - 1) ? L'┘' : L'─';
      pixels.push_back(Pixel(Position(x, bottomY), ch, borderColor, normalBgColor, A_NORMAL));
   }

   // Add scroll indicators if needed
   if (m_scrollOffset > 0)
   {
      // Up arrow indicator
      pixels.push_back(Pixel(Position(m_menuWidth - 2, 1), L'▲', borderColor, normalBgColor, A_NORMAL));
   }

   if (m_scrollOffset + m_maxVisibleItems < (int)m_menuItems.size())
   {
      // Down arrow indicator
      pixels.push_back(
            Pixel(Position(m_menuWidth - 2, visibleItemCount), L'▼', borderColor, normalBgColor, A_NORMAL));
   }

   // Create sprite and animation
   Sprite menuSprite(pixels, 1);

   // Preserve the anchor position to maintain centering
   menuSprite.moveAnchorToPosition(currentAnchor);

   Frame              menuFrame(menuSprite, 10.0f); // Long duration since we update manually
   std::vector<Frame> frames = {menuFrame};
   Animation          menuAnimation("menu", frames, false);

   // Clear existing animations and add new one
   m_animations.clear();
   m_animations.push_back(menuAnimation);
   m_currentAnimationName = "menu";
}