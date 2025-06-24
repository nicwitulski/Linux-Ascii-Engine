#include "../../../include/UIManager.h"

void UIManager::registerElement(UIElement *element,
                                ScreenLockPositions position) {
  m_lockedElements[position].push_back(element);
}

void UIManager::updatePositions() 
{
    
}