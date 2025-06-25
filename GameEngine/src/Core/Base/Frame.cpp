#include "../../../include/Frame.h"

Frame::Frame() {
  m_sprite = Sprite();
  m_durationInSeconds = 1.0f;
};

Frame::Frame(Sprite sprite, float duration) {
  m_sprite = sprite;
  m_durationInSeconds = duration;
};

float Frame::getDuration() { return m_durationInSeconds; };

Sprite &Frame::getSprite() { return m_sprite; };

void Frame::setSprite(Sprite sprite) { m_sprite = sprite; };

void Frame::displace(int dx, int dy) { m_sprite.displace(dx, dy); }