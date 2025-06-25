#ifndef FRAME_H
#define FRAME_H

#include "Sprite.h"

class Frame {
private:
  Sprite m_sprite;
  float m_durationInSeconds;

public:
  Frame();
  Frame(Sprite sprite, float duration);
  float getDuration();
  Sprite &getSprite();
  void setSprite(Sprite sprite);
  void displace(int dx, int dy);
};

#endif