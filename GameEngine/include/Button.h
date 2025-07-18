//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file Button.h
/// @author Nicholas Witulski (nicwitulski@gmail.com)
/// @brief Defines a button with an executable function
/// @version 0.1
/// @date 2025-06-27
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef BUTTON_H
#define BUTTON_H

#include "UIElement.h"
#include <functional>
#include <vector>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class Button
///
/// Defines a button with an executable function
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class Button : public UIElement
{
private:
   std::function<void()> m_function;
   std::vector<RGB>      m_originalBackgroundColors;
   std::vector<Position> m_originalPositions;
   bool                  m_isHighlighted;

   // Automatic highlighting configuration
   bool m_autoHighlightEnabled;
   RGB  m_hoverColor;
   RGB  m_clickColor;
   RGB  m_selectedColor;

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn storeOriginalColors
   ///
   /// Stores the original background colors of all pixels for unhighlighting
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   void storeOriginalColors();

public:
   virtual ~Button();
   Button();
   Button(const std::string printableName, const std::vector<Animation> animations, const bool visable,
          const bool moveableByCamera, std::function<void()> function);
   Button(const std::string printableName, const std::vector<Animation> animations, const bool visable,
          const bool moveableByCamera);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn setFunction
   ///
   /// @param func - function to set when executeFuntion() is called
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   void setFunction(std::function<void()> func);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn executeFunction
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   void executeFunction();

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn hasFunction
   ///
   /// @return whether the button has a function assigned
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   bool hasFunction() const;

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn mouseInBounds
   ///
   /// @param position - position of mouse to check bounds of
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   bool mouseInBounds(Position position);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn setText
   ///
   /// Sets the button text and wraps the outline around the new text
   /// @param text - the new text to display on the button
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   void setText(std::string text);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn highlight
   ///
   /// Changes all pixels' background colors to the specified RGB value
   /// @param rgbValue - the RGB color to apply to all pixel backgrounds
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   void highlight(RGB rgbValue);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn unhighlight
   ///
   /// Restores all pixels' background colors to their original values
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   void unhighlight();

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn setAutoHighlightEnabled
   ///
   /// Enables or disables automatic highlighting on hover/click
   /// @param enabled - whether to enable automatic highlighting
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   void setAutoHighlightEnabled(bool enabled);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn isAutoHighlightEnabled
   ///
   /// @return whether automatic highlighting is enabled
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   bool isAutoHighlightEnabled() const;

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn setHoverColor
   ///
   /// Sets the color used when the mouse hovers over the button
   /// @param color - RGB color for hover highlighting
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   void setHoverColor(const RGB& color);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn setClickColor
   ///
   /// Sets the color used when the button is clicked/pressed
   /// @param color - RGB color for click highlighting
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   void setClickColor(const RGB& color);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn setSelectedColor
   ///
   /// Sets the color used when the button is selected/active
   /// @param color - RGB color for selected highlighting
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   void setSelectedColor(const RGB& color);

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn getHoverColor
   ///
   /// @return the RGB color used for hover highlighting
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   const RGB& getHoverColor() const;

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn getClickColor
   ///
   /// @return the RGB color used for click highlighting
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   const RGB& getClickColor() const;

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   /// @fn getSelectedColor
   ///
   /// @return the RGB color used for selected highlighting
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////
   const RGB& getSelectedColor() const;
};

#endif