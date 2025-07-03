//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file UIElement.h
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Defines a UI element and gives option to lock element to edge/corner of terminal
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef UIELEMENT_H
#define UIELEMENT_H

#include "Printable.h"
#include <map>
#include <memory>
#include <vector>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class ScreenLockPosition
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class ScreenLockPosition
{
   NONE,
   TOP_LEFT_CORNER,
   TOP_MIDDLE,
   TOP_RIGHT_CORNER,
   RIGHT_MIDDLE,
   BOTTOM_RIGHT_CORNER,
   BOTTOM_MIDDLE,
   BOTTOM_LEFT_CORNER,
   LEFT_MIDDLE,
   CENTER
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class StackDirection
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum class StackDirection
{
   VERTICAL,
   HORIZONTAL
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class UIElement
///
/// Defines an UIElement. Edge locking is mostly vibe coded....
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class UIElement : public std::enable_shared_from_this<UIElement>, public Printable
{
protected:
   Position           m_maxPosition;
   Position           m_minPosition;
   ScreenLockPosition m_lockPosition;
   StackDirection     m_stackDirection;

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn setPositions
   ///
   /// Sets the max and min position based on the entity
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   void setPositions();

public:
   UIElement();
   UIElement(const std::string printableName, const std::vector<Animation> animations, const bool visable,
             const bool moveableByCamera);

   static std::vector<std::shared_ptr<UIElement>> topMiddleUIElements;
   static std::vector<std::shared_ptr<UIElement>> rightMiddleUIElements;
   static std::vector<std::shared_ptr<UIElement>> bottomMiddleUIElements;
   static std::vector<std::shared_ptr<UIElement>> leftMiddleUIElements;
   static std::vector<std::shared_ptr<UIElement>> topLeftUIElements;
   static std::vector<std::shared_ptr<UIElement>> topRightUIElements;
   static std::vector<std::shared_ptr<UIElement>> bottomRightUIElements;
   static std::vector<std::shared_ptr<UIElement>> bottomLeftUIElements;
   static std::vector<std::shared_ptr<UIElement>> middleUIElements;

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn setDynamicPosition
   ///
   /// @param position - Locks the element to this terminal edge/corner
   /// @param direction - if in a corner, can set the dyanamic stacking direction to be vertical or horizontal
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   void setDynamicPosition(ScreenLockPosition position, StackDirection direction = StackDirection::VERTICAL);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn updateAllLockedPositions
   ///
   /// Updates all elements. Gets called every time the terminal gets resized
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   static void updateAllLockedPositions();

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn displace
   ///
   /// @param dx - x axis difference
   /// @param dy - y axis difference
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   void displace(int dx, int dy) override;

   std::shared_ptr<UIElement> getptr() { return shared_from_this(); }
};

#endif