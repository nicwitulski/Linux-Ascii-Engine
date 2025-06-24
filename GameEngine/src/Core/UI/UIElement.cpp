#include "../../../include/UIElement.h"
#include "../../../include/Parameters.h"

// Define all static member variables
std::vector<std::shared_ptr<UIElement>> UIElement::topMiddleUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::rightMiddleUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::bottomMiddleUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::leftMiddleUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::middleUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::topLeftUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::topRightUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::bottomLeftUIElements;
std::vector<std::shared_ptr<UIElement>> UIElement::bottomRightUIElements;

UIElement::UIElement() {
  m_minPosition = Position(0, 0);
  m_maxPosition = Position(0, 0);
  m_entity = nullptr;
  m_lockPosition = ScreenLockPosition::NONE;
}

UIElement::UIElement(std::shared_ptr<Entity> entity) {
  m_entity = entity;
  m_lockPosition = ScreenLockPosition::NONE;
  setPositions();
}

void UIElement::setPositions() {
  m_minPosition = m_entity->getAnchor();
  int maxX = 0;
  int maxY = 0;

  for (Animation animation : m_entity->getAnimations()) {
    if (animation.getAnimationName() == m_entity->getCurrentAnimationName()) {
      for (Pixel pixel : animation.getCurrentFrameSprite().getPixels()) {
        if (pixel.getPosition().getX() > maxX)
          maxX = pixel.getPosition().getX();
        if (pixel.getPosition().getY() > maxY)
          maxY = pixel.getPosition().getY();
      }
    }
  }

  m_maxPosition = Position(maxX, maxY);
}

void UIElement::setDynamicPosition(ScreenLockPosition position,
                                   StackDirection direction) {
  m_lockPosition = position;
  m_stackDirection = direction;
  switch (m_lockPosition) {
  case ScreenLockPosition::TOP_MIDDLE:
    topMiddleUIElements.push_back(shared_from_this());
    break;
  case ScreenLockPosition::RIGHT_MIDDLE:
    rightMiddleUIElements.push_back(shared_from_this());
    break;
  case ScreenLockPosition::BOTTOM_MIDDLE:
    bottomMiddleUIElements.push_back(shared_from_this());
    break;
  case ScreenLockPosition::LEFT_MIDDLE:
    leftMiddleUIElements.push_back(shared_from_this());
    break;
  case ScreenLockPosition::CENTER:
    middleUIElements.push_back(shared_from_this());
    break;
  case ScreenLockPosition::TOP_LEFT_CORNER:
    topLeftUIElements.push_back(shared_from_this());
    break;
  case ScreenLockPosition::TOP_RIGHT_CORNER:
    topRightUIElements.push_back(shared_from_this());
    break;
  case ScreenLockPosition::BOTTOM_LEFT_CORNER:
    bottomLeftUIElements.push_back(shared_from_this());
    break;
  case ScreenLockPosition::BOTTOM_RIGHT_CORNER:
    bottomRightUIElements.push_back(shared_from_this());
    break;

  default:
    break;
  }
  m_stackDirection = direction;
}

void UIElement::updateAllLockedPositions() {
  // Helper lambdas
  auto elementWidth = [](const std::shared_ptr<UIElement> &el) {
    return el->m_maxPosition.getX() - el->m_minPosition.getX() + 1;
  };
  auto elementHeight = [](const std::shared_ptr<UIElement> &el) {
    return el->m_maxPosition.getY() - el->m_minPosition.getY() + 1;
  };

  // --- TOP MIDDLE ---
  {
    int totalWidth = 0;
    for (auto &el : topMiddleUIElements) {
      el->setPositions();
      el->m_entity->moveToPosition(Position(0, 0));
      el->setPositions();
      totalWidth += elementWidth(el);
    }
    int dx = (SCREEN_LENGTH - totalWidth) / 2;
    for (auto &el : topMiddleUIElements) {
      el->m_entity->moveToPosition(Position(dx, 0));
      el->setPositions();
      dx += elementWidth(el);
    }
  }

  // --- RIGHT MIDDLE ---
  {
    int totalHeight = 0;
    for (auto &el : rightMiddleUIElements) {
      el->setPositions();
      el->m_entity->moveToPosition(Position(0, 0));
      el->setPositions();
      totalHeight += elementHeight(el);
    }
    int dy = (SCREEN_HEIGHT - totalHeight) / 2;
    for (auto &el : rightMiddleUIElements) {
      int x = SCREEN_LENGTH - elementWidth(el);
      el->m_entity->moveToPosition(Position(x, dy));
      el->setPositions();
      dy += elementHeight(el);
    }
  }

  // --- BOTTOM MIDDLE ---
  {
    int totalWidth = 0;
    for (auto &el : bottomMiddleUIElements) {
      el->setPositions();
      el->m_entity->moveToPosition(Position(0, 0));
      el->setPositions();
      totalWidth += elementWidth(el);
    }
    int dx = (SCREEN_LENGTH - totalWidth) / 2;
    int y = SCREEN_HEIGHT - 1; // bottom row
    for (auto &el : bottomMiddleUIElements) {
      el->m_entity->moveToPosition(Position(dx, y - elementHeight(el) + 1));
      el->setPositions();
      dx += elementWidth(el);
    }
  }

  // --- LEFT MIDDLE ---
  {
    int totalHeight = 0;
    for (auto &el : leftMiddleUIElements) {
      el->setPositions();
      el->m_entity->moveToPosition(Position(0, 0));
      el->setPositions();
      totalHeight += elementHeight(el);
    }
    int dy = (SCREEN_HEIGHT - totalHeight) / 2;
    for (auto &el : leftMiddleUIElements) {
      el->m_entity->moveToPosition(Position(0, dy));
      el->setPositions();
      dy += elementHeight(el);
    }
  }

  // --- CENTER ---
  {
    int totalWidth = 0;
    int maxHeight = 0;
    for (auto &el : middleUIElements) {
      el->setPositions();
      el->m_entity->moveToPosition(Position(0, 0));
      el->setPositions();
      totalWidth += elementWidth(el);
      if (elementHeight(el) > maxHeight)
        maxHeight = elementHeight(el);
    }
    int dx = (SCREEN_LENGTH - totalWidth) / 2;
    int y = (SCREEN_HEIGHT - maxHeight) / 2;
    for (auto &el : middleUIElements) {
      el->m_entity->moveToPosition(Position(dx, y));
      el->setPositions();
      dx += elementWidth(el);
    }
  }

  // --- TOP LEFT CORNER ---
  {
    int dx = 0;
    int dy = 0;
    for (auto &el : topLeftUIElements) {
      el->setPositions();
      el->m_entity->moveToPosition(Position(dx, dy));
      el->setPositions();

      if (el->m_stackDirection == StackDirection::HORIZONTAL) {
        dx += elementWidth(el);
      } else {
        dy += elementHeight(el);
      }
    }
  }

  // --- TOP RIGHT CORNER ---
  {
    int dx = SCREEN_LENGTH;
    int dy = 0;
    for (auto &el : topRightUIElements) {
      el->setPositions();

      if (el->m_stackDirection == StackDirection::HORIZONTAL) {
        dx -= elementWidth(el);
        el->m_entity->moveToPosition(Position(dx, dy));
        dx -= 0; // no-op
      } else {
        el->m_entity->moveToPosition(Position(dx - elementWidth(el), dy));
        dy += elementHeight(el);
      }

      el->setPositions();
    }
  }

  // --- BOTTOM LEFT CORNER ---
  {
    int dx = 0;
    int dy = SCREEN_HEIGHT;
    for (auto &el : bottomLeftUIElements) {
      el->setPositions();

      if (el->m_stackDirection == StackDirection::HORIZONTAL) {
        el->m_entity->moveToPosition(Position(dx, dy - elementHeight(el)));
        dx += elementWidth(el);
      } else {
        dy -= elementHeight(el);
        el->m_entity->moveToPosition(Position(dx, dy));
      }

      el->setPositions();
    }
  }

  // --- BOTTOM RIGHT CORNER ---
  {
    int dx = SCREEN_LENGTH;
    int dy = SCREEN_HEIGHT;
    for (auto &el : bottomRightUIElements) {
      el->setPositions();

      if (el->m_stackDirection == StackDirection::HORIZONTAL) {
        dx -= elementWidth(el);
        el->m_entity->moveToPosition(Position(dx, dy - elementHeight(el)));
      } else {
        dy -= elementHeight(el);
        el->m_entity->moveToPosition(Position(dx - elementWidth(el), dy));
      }

      el->setPositions();
    }
  }
}