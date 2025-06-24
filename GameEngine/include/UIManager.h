#ifndef UIMANAGER_H
#define UIMANAGER_H

#include "UIElement.h"
#include <map>
#include <vector>

class UIManager {
private:
  enum class ScreenLockPositions {
    TOP_RIGHT,
    TOP_MIDDLE,
    TOP_LEFT,
    LEFT_MIDDLE,
    MIDDLE,
    RIGHT_MIDDLE,
    BOTTOM_RIGHT,
    BOTTOM_MIDDLE,
    BOTTOM_LEFT
  };

  std::map<ScreenLockPositions, std::vector<UIElement *>> m_lockedElements;

public:
  void registerElement(UIElement *element, ScreenLockPositions position);
  void updatePositions();
};

#endif