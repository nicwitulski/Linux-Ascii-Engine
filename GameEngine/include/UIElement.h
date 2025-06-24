#ifndef UIELEMENT_H
#define UIELEMENT_H

#include "Entity.h"
#include <map>
#include <memory>
#include <vector>

enum class ScreenLockPosition {
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

enum class StackDirection { VERTICAL, HORIZONTAL };

class UIElement : public std::enable_shared_from_this<UIElement> {
protected:
  Position m_maxPosition;
  Position m_minPosition;
  std::shared_ptr<Entity> m_entity;
  ScreenLockPosition m_lockPosition;
  StackDirection m_stackDirection;

  void setPositions();
  void updateElement();

public:
  UIElement();
  UIElement(std::shared_ptr<Entity> entity);

  static std::vector<std::shared_ptr<UIElement>> topMiddleUIElements;
  static std::vector<std::shared_ptr<UIElement>> rightMiddleUIElements;
  static std::vector<std::shared_ptr<UIElement>> bottomMiddleUIElements;
  static std::vector<std::shared_ptr<UIElement>> leftMiddleUIElements;
  static std::vector<std::shared_ptr<UIElement>> topLeftUIElements;
  static std::vector<std::shared_ptr<UIElement>> topRightUIElements;
  static std::vector<std::shared_ptr<UIElement>> bottomRightUIElements;
  static std::vector<std::shared_ptr<UIElement>> bottomLeftUIElements;
  static std::vector<std::shared_ptr<UIElement>> middleUIElements;

  void setDynamicPosition(ScreenLockPosition position,
                          StackDirection direction = StackDirection::VERTICAL);
  static void updateAllLockedPositions();

  std::shared_ptr<UIElement> getptr() { return shared_from_this(); }
};

#endif
