#ifndef UIELEMENT_H
#define UIELEMENT_H

#include "Entity.h"
#include <memory>

class UIElement {
protected:
  Position m_maxPosition;
  Position m_minPosition;
  std::shared_ptr<Entity> m_entity;

  void setPositions();

public:
  UIElement();
  UIElement(std::shared_ptr<Entity> entity);
};

#endif