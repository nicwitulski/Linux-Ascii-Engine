#include "../../include/Display.h"
#include <ncurses.h>

void Display::initCurse() {
  initscr(); // Start curses mode
  curs_set(0);
  noecho();              // Don't echo keypresses
  cbreak();              // Disable line buffering
  keypad(stdscr, TRUE);  // Enable function keys and arrow keys
  nodelay(stdscr, TRUE); // Non-blocking getch
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION,
            NULL);  // Enable mouse events
  mouseinterval(0); // No delay for mouse clicks

  printf("\033[?1003h\n"); // Enable mouse tracking in xterm
};

char Display::getUserInput() { return getch(); }

void Display::closeCurseWindow() {
  printf("\033[?1003l\n"); // Disable mouse tracking
  endwin();
};

void Display::refreshDisplay(float deltaTime) {
  int tempHeight, tempLength;
  getmaxyx(stdscr, tempHeight, tempLength);
  // Terminal size change
  if (tempHeight != SCREEN_HEIGHT || tempLength != SCREEN_LENGTH) {
    SCREEN_HEIGHT = tempHeight;
    SCREEN_LENGTH = tempLength;
    currentCamera->setHeight(SCREEN_HEIGHT);
    currentCamera->setLength(SCREEN_LENGTH);
    UIElement::updateAllLockedPositions();
  }

  if (displayNeedsCleared) {
    clear();
    displayNeedsCleared = false;
  }

  refreshEntities(deltaTime);
  wnoutrefresh(stdscr);
  doupdate();
  curs_set(0);
}

void Display::printPixel(Pixel pixel, bool isMoveableByCamera) {
  int printedX = 0;
  int printedY = 0;

  if (isMoveableByCamera) {
    printedX = pixel.getPosition().getX() + currentCamera->getLengthOffset();
    printedY = pixel.getPosition().getY() + currentCamera->getHeightOffset();
  } else {
    printedX = pixel.getPosition().getX();
    printedY = pixel.getPosition().getY();
  }

  if (printedX >= 0 && printedX <= currentCamera->getLength() &&
      printedY >= 0 && printedY <= currentCamera->getHeight()) {

    mvprintw(printedY, printedX, "%c", pixel.getCharacter());
  }
};

void Display::printSprite(Sprite sprite, bool isMoveableByCamera) {
  for (std::size_t i = 0; i < sprite.getPixels().size(); i++) {
    Pixel pixel = sprite.getPixels().at(i);
    printPixel(pixel, isMoveableByCamera);
  }
};

void Display::eraseSprite(Sprite sprite, bool isMoveableByCamera) {
  for (Pixel &pixel : sprite.getPixels()) {
    printPixel(Pixel(pixel.getPosition(), ' '), isMoveableByCamera);
  }
}

void Display::refreshEntities(float deltaTime) {
  if (printablesNeedSorted) {
    std::sort(allPrintables.begin(), allPrintables.end(),
              [](const std::shared_ptr<Printable> &a,
                 const std::shared_ptr<Printable> &b) {
                return a->getLayer() < b->getLayer();
              });
    printablesNeedSorted = false;
  }

  for (auto &printable : allPrintables) {
    for (Animation &animation : printable->getAnimations()) {
      if (animation.getAnimationName() ==
          printable->getCurrentAnimationName()) {
        if (!printable->isStatic() && animation.isPlaying()) {
          animation.update(deltaTime);
          printable->addDirtySprite(animation.getPreviousFrameSprite());
        }

        for (Sprite sprite : printable->getDirtySprites()) {
          eraseSprite(sprite, printable->isMoveableByCamera());
        }
        printable->getDirtySprites().clear();
        printSprite(animation.getCurrentFrameSprite(),
                    printable->isMoveableByCamera());
      }
    }
  }
}