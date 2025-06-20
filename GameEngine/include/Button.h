#ifndef BUTTON_H
#define BUTTON_H

#include "UIElement.h"
#include <functional>

class Button : public UIElement {
private:
  std::function<void()> m_function;
  void setBoundsBasedOnEntity();

public:
  Button();
  Button(std::shared_ptr<Entity> entity, std::function<void()> function);
  void setFunction(std::function<void()> func);
  void executeFunction();
  bool mouseInBounds(int x, int y);
  void displace(int dx, int dy);
};

#endif