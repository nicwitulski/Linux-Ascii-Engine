#include "../../../include/Camera.h"
#include "../../../include/Parameters.h"

Camera::Camera() {
  m_length = 80;
  m_height = 24;
  m_lengthOffset = 0;
  m_heightOffset = 0;
  m_lastLengthOffset = 0;
  m_lastHeightOffset = 0;
};

Camera::Camera(int length, int height) {
  m_length = length;
  m_height = height;
  m_lengthOffset = 0;
  m_heightOffset = 0;
  m_lastLengthOffset = 0;
  m_lastHeightOffset = 0;
};

void Camera::displaceViewPort(int dx, int dy) {
  if (dx != 0 || dy != 0) {
    displayNeedsCleared = true;
  }
  m_lengthOffset += dx;
  m_heightOffset += dy;
}

int Camera::getLength() { return m_length; }
void Camera::setLength(int length) { m_length = length; }

int Camera::getHeight() { return m_height; }
void Camera::setHeight(int height) { m_height = height; }

int Camera::getHeightOffset() { return m_heightOffset; }

int Camera::getLengthOffset() { return m_lengthOffset; }