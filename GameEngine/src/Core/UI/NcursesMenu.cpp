//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file NcursesMenu.cpp
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Implementation of native ncurses menu with built-in highlighting
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../../include/NcursesMenu.h"
#include <cstring>
#include <iostream>

// public ----------------------------------------------------------------------------------------------------
NcursesMenu::NcursesMenu(const std::vector<std::string>& menuItems, int windowWidth, int windowHeight,
                         int maxVisibleItems)
   : m_items(nullptr),
     m_menu(nullptr),
     m_menuWindow(nullptr),
     m_subWindow(nullptr),
     m_menuItems(menuItems),
     m_isActive(false),
     m_windowWidth(windowWidth),
     m_windowHeight(windowHeight),
     m_maxVisibleItems(maxVisibleItems)
{
   createMenuItems();
}

// public ----------------------------------------------------------------------------------------------------
NcursesMenu::~NcursesMenu()
{
   cleanupMenu();
}

// public ----------------------------------------------------------------------------------------------------
void NcursesMenu::setPosition(int x, int y)
{
   if (m_menuWindow)
   {
      mvwin(m_menuWindow, y, x);
   }
}

// public ----------------------------------------------------------------------------------------------------
void NcursesMenu::setOnSelectCallback(std::function<void(const std::string&)> callback)
{
   m_onSelectCallback = callback;
}

// public ----------------------------------------------------------------------------------------------------
void NcursesMenu::setOnCancelCallback(std::function<void()> callback)
{
   m_onCancelCallback = callback;
}

// public ----------------------------------------------------------------------------------------------------
bool NcursesMenu::handleInput(int key)
{
   if (!m_isActive || !m_menu)
   {
      return false;
   }

   switch (key)
   {
      case KEY_UP:
         menu_driver(m_menu, REQ_UP_ITEM);
         refresh();
         return true;
      case KEY_DOWN:
         menu_driver(m_menu, REQ_DOWN_ITEM);
         refresh();
         return true;
      case KEY_NPAGE:
         menu_driver(m_menu, REQ_SCR_DPAGE);
         refresh();
         return true;
      case KEY_PPAGE:
         menu_driver(m_menu, REQ_SCR_UPAGE);
         refresh();
         return true;
      case '\n':
      case '\r':
         if (m_onSelectCallback)
         {
            std::string selectedItem = getSelectedItem();
            if (!selectedItem.empty())
            {
               m_onSelectCallback(selectedItem);
            }
         }
         return true;
      case 27: // ESC key
         if (m_onCancelCallback)
         {
            m_onCancelCallback();
         }
         return true;
      default:
         return false;
   }
}

// public ----------------------------------------------------------------------------------------------------
void NcursesMenu::show()
{
   if (!m_menu)
   {
      return;
   }

   // Calculate centered position
   int screenWidth, screenHeight;
   getmaxyx(stdscr, screenHeight, screenWidth);
   int startX = (screenWidth - m_windowWidth) / 2;
   int startY = (screenHeight - m_windowHeight) / 2;

   // Create the main window
   m_menuWindow = newwin(m_windowHeight, m_windowWidth, startY, startX);
   keypad(m_menuWindow, TRUE);

   // Make sure the window is on top and visible
   wattron(m_menuWindow, A_BOLD);
   wbkgd(m_menuWindow, COLOR_PAIR(0));

   // Create the sub-window for the menu items
   m_subWindow = derwin(m_menuWindow, m_windowHeight - 4, m_windowWidth - 4, 2, 2);

   // Set the menu window and sub-window
   set_menu_win(m_menu, m_menuWindow);
   set_menu_sub(m_menu, m_subWindow);

   // Set menu format (rows, columns)
   set_menu_format(m_menu, m_maxVisibleItems, 1);

   // Set menu mark
   set_menu_mark(m_menu, " > ");

   // Draw border and title
   box(m_menuWindow, 0, 0);
   mvwprintw(m_menuWindow, 1, 2, "Select Animation:");
   mvwaddch(m_menuWindow, 2, 0, ACS_LTEE);
   mvwhline(m_menuWindow, 2, 1, ACS_HLINE, m_windowWidth - 2);
   mvwaddch(m_menuWindow, 2, m_windowWidth - 1, ACS_RTEE);

   // Post the menu
   post_menu(m_menu);

   // Refresh both windows
   wrefresh(m_menuWindow);
   wrefresh(m_subWindow);

   // Make sure the menu window gets focus for input
   wmove(m_menuWindow, 0, 0);

   m_isActive = true;
}

// public ----------------------------------------------------------------------------------------------------
void NcursesMenu::hide()
{
   m_isActive = false;

   if (m_menu)
   {
      unpost_menu(m_menu);
   }

   if (m_menuWindow)
   {
      werase(m_menuWindow);
      wrefresh(m_menuWindow);
      delwin(m_menuWindow);
      m_menuWindow = nullptr;
   }

   if (m_subWindow)
   {
      delwin(m_subWindow);
      m_subWindow = nullptr;
   }

   // Force a complete screen refresh to clear the menu
   clear();
   refresh();
   touchwin(stdscr);
   wrefresh(stdscr);
}

// public ----------------------------------------------------------------------------------------------------
bool NcursesMenu::isActive() const
{
   return m_isActive;
}

// public ----------------------------------------------------------------------------------------------------
void NcursesMenu::refresh()
{
   if (m_menuWindow && m_isActive)
   {
      // Redraw the border and title
      box(m_menuWindow, 0, 0);
      mvwprintw(m_menuWindow, 1, 2, "Select Animation:");
      mvwaddch(m_menuWindow, 2, 0, ACS_LTEE);
      mvwhline(m_menuWindow, 2, 1, ACS_HLINE, m_windowWidth - 2);
      mvwaddch(m_menuWindow, 2, m_windowWidth - 1, ACS_RTEE);

      // Refresh both windows
      wrefresh(m_menuWindow);
      if (m_subWindow)
      {
         wrefresh(m_subWindow);
      }
   }
}

// public ----------------------------------------------------------------------------------------------------
std::string NcursesMenu::getSelectedItem() const
{
   if (!m_menu)
   {
      return "";
   }

   ITEM* currentItem = current_item(m_menu);
   if (currentItem)
   {
      return std::string(item_name(currentItem));
   }

   return "";
}

// private ---------------------------------------------------------------------------------------------------
void NcursesMenu::createMenuItems()
{
   if (m_menuItems.empty())
   {
      return;
   }

   // Allocate memory for items array (+1 for NULL terminator)
   m_items = new ITEM*[m_menuItems.size() + 1];

   // Create items
   for (size_t i = 0; i < m_menuItems.size(); ++i)
   {
      // Create a persistent copy of the string
      char* itemName = new char[m_menuItems[i].length() + 1];
      std::strcpy(itemName, m_menuItems[i].c_str());
      m_items[i] = new_item(itemName, nullptr);
   }

   // NULL terminate the array
   m_items[m_menuItems.size()] = nullptr;

   // Create the menu
   m_menu = new_menu(m_items);
}

// private ---------------------------------------------------------------------------------------------------
void NcursesMenu::cleanupMenu()
{
   if (m_menu)
   {
      unpost_menu(m_menu);
      free_menu(m_menu);
      m_menu = nullptr;
   }

   if (m_items)
   {
      for (size_t i = 0; i < m_menuItems.size(); ++i)
      {
         if (m_items[i])
         {
            // Free the item name that we allocated
            char* itemName = const_cast<char*>(item_name(m_items[i]));
            delete[] itemName;
            free_item(m_items[i]);
         }
      }
      delete[] m_items;
      m_items = nullptr;
   }

   if (m_menuWindow)
   {
      delwin(m_menuWindow);
      m_menuWindow = nullptr;
   }

   if (m_subWindow)
   {
      delwin(m_subWindow);
      m_subWindow = nullptr;
   }
}