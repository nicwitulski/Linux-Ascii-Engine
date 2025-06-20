#include "../../../include/UIElement.h"

UIElement::UIElement() {
  m_minPosition = Position(0, 0);
  m_maxPosition = Position(0, 0);
  m_entity = nullptr;
};

UIElement::UIElement(std::shared_ptr<Entity> entity) {
  m_entity = entity;
  setPositions();
};

void UIElement::setPositions() {
  m_minPosition = m_entity->getAnchor();
  int maxX = 0;
  int maxY = 0;

  for (Animation animation : m_entity->getAnimations()) {
    if (animation.getAnimationName() == m_entity->getCurrentAnimationName()) {
      for (Pixel pixel : animation.getCurrentFrameSprite().getPixels()) {
        if (pixel.getPosition().getX() > maxX) {
          maxX = pixel.getPosition().getX();
          if (pixel.getPosition().getY() > maxY) {
            maxY = pixel.getPosition().getY();
          }
        }
      }
    }
  }
};