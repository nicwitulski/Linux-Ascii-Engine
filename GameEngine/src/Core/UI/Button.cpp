#include "../../../include/Button.h"

Button::Button() {
  m_entity = nullptr;
  m_minPosition = Position(0, 0);
  m_maxPosition = Position(0, 0);
};

Button::Button(std::shared_ptr<Entity> entity, std::function<void()> function) {
  m_minPosition = Position(0, 0);
  m_maxPosition = Position(0, 0);
  m_entity = entity;
  setFunction(function);
  setBoundsBasedOnEntity();
}

void Button::setFunction(std::function<void()> func) {
  m_function = std::move(func);
};

void Button::executeFunction() {
  if (m_function) {
    m_function();
  } else {
    { return; }
  }
};

bool Button::mouseInBounds(int x, int y) {
  // std::cout << m_minPosition.getX() << ", " << m_minPosition.getY() <<
  // std::endl
  //           << m_maxPosition.getX() << ", " << m_maxPosition.getY()
  //           << std::endl;
  if (x >= m_minPosition.getX() && x <= m_maxPosition.getX() &&
      y >= m_minPosition.getY() && y <= m_maxPosition.getY()) {
    return true;
  }
  return false;
}

void Button::displace(int dx, int dy) {
  m_minPosition =
      Position(m_minPosition.getX() + dx, m_minPosition.getY() + dy);
  m_maxPosition =
      Position(m_maxPosition.getX() + dx, m_maxPosition.getY() + dy);
  m_entity->displace(dx, dy);
}

void Button::setBoundsBasedOnEntity() {
  int minX = m_minPosition.getX();
  int maxX = m_maxPosition.getX();
  int minY = m_minPosition.getY();
  int maxY = m_maxPosition.getY();

  for (Frame frames : m_entity->getCurrentAnimation().getFrames()) {
    for (Pixel pixel : frames.getSprite().getPixels()) {
      int x = pixel.getPosition().getX();
      int y = pixel.getPosition().getY();

      if (x < minX) {
        minX = x;
      } else if (x > maxX) {
        maxX = x;
      }

      if (y < minY) {
        minY = y;
      } else if (y > maxY) {
        maxY = y;
      }
    }
  }
  m_minPosition = Position(minX, minY);
  m_maxPosition = Position(maxX, maxY);
}