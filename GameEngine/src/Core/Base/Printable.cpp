#include "../../../include/Printable.h"
#include "../../../include/Animation.h"

Printable::Printable() {
  m_currentAnimation = "default";
  m_animations.push_back(Animation());
  m_visable = false;
  m_layer = 0;
  m_anchor = Position(0, 0);
  m_static = false;
}

void Printable::addAnimation(const Animation animation) {
  m_animations.push_back(animation);
};

bool Printable::setCurrentAnimation(const std::string name) {
  for (Animation animation : m_animations) {
    if (animation.getAnimationName() == name) {
      m_currentAnimation = animation.getAnimationName();
      return true;
    }
  }
  return false; // If not loaded, stays as current animation
};

std::string Printable::getCurrentAnimationName() { return m_currentAnimation; };

std::vector<Animation> &Printable::getAnimations() { return m_animations; };

Position Printable::getAnchor() { return m_anchor; }

void Printable::displace(int dx, int dy) {
  for (Animation &animation : m_animations) {
    if (m_currentAnimation == animation.getAnimationName()) {
      m_dirtySprites.push_back(animation.getCurrentFrameSprite());
      animation.displace(dx, dy);
      m_anchor = Position(m_anchor.getX() + dx, m_anchor.getY() + dy);
      break;
    }
  }
};

void Printable::moveToPosition(Position position) {
  int difX = position.getX() - m_anchor.getX();
  int difY = position.getY() - m_anchor.getY();
  displace(difX, difY);
  m_anchor = position;
};

bool Printable::isVisable() { return m_visable; };

int Printable::getLayer() { return m_layer; };

void Printable::setVisability(bool visable) { m_visable = visable; };

void Printable::setLayer(int layer) { m_layer = layer; }

bool Printable::isMoveableByCamera() { return m_moveableByCamera; };

void Printable::setMoveableByCamera(bool moveableByCamera) {
  m_moveableByCamera = moveableByCamera;
};

Animation &Printable::getCurrentAnimation() {
  for (Animation &animation : m_animations) {
    if (m_currentAnimation == animation.getAnimationName()) {
      return animation;
    }
  }
  return m_animations.at(0);
}

bool Printable::isStatic() { return m_static; }

void Printable::setStatic(bool staticAni) { m_static = staticAni; }

std::vector<Sprite> &Printable::getDirtySprites() { return m_dirtySprites; }

void Printable::addDirtySprite(Sprite dirtySprite) {
  m_dirtySprites.push_back(dirtySprite);
}