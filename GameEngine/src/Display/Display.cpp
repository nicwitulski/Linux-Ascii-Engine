#include "../../include/Display.h"
#include <ncurses.h>

// Initialize static members
std::vector<std::vector<char>> Display::currentFrameBuffer;
std::vector<std::vector<char>> Display::lastFrameBuffer;

void Display::initCurse() {
  setlocale(LC_ALL, "");
  initscr(); // Start curses mode
  curs_set(0);
  noecho();
  cbreak();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  mouseinterval(0);
  printf("\033[?1003h\n"); // Enable mouse tracking

  getmaxyx(stdscr, SCREEN_HEIGHT, SCREEN_LENGTH);
  currentFrameBuffer = std::vector<std::vector<char>>(
      SCREEN_HEIGHT, std::vector<char>(SCREEN_LENGTH, ' '));
  lastFrameBuffer = currentFrameBuffer;
}

char Display::getUserInput() { return getch(); }

void Display::closeCurseWindow() {
  printf("\033[?1003l\n"); // Disable mouse tracking
  endwin();
}

void Display::refreshDisplay(float deltaTime) {
  int tempHeight, tempLength;
  getmaxyx(stdscr, tempHeight, tempLength);

  if (tempHeight != SCREEN_HEIGHT || tempLength != SCREEN_LENGTH) {
    SCREEN_HEIGHT = tempHeight;
    SCREEN_LENGTH = tempLength;

    currentCamera->setHeight(SCREEN_HEIGHT);
    currentCamera->setLength(SCREEN_LENGTH);
    UIElement::updateAllLockedPositions();

    currentFrameBuffer = std::vector<std::vector<char>>(
        SCREEN_HEIGHT, std::vector<char>(SCREEN_LENGTH, ' '));
    lastFrameBuffer = currentFrameBuffer;
    displayNeedsCleared = true;
  }

  if (displayNeedsCleared) {
    clear();
    for (auto &row : currentFrameBuffer)
      std::fill(row.begin(), row.end(), ' ');
    for (auto &row : lastFrameBuffer)
      std::fill(row.begin(), row.end(), ' ');
    displayNeedsCleared = false;
  }

  refreshEntities(deltaTime);

  // Draw diffs
  for (int y = 0; y < SCREEN_HEIGHT; ++y) {
    for (int x = 0; x < SCREEN_LENGTH; ++x) {
      if (currentFrameBuffer[y][x] != lastFrameBuffer[y][x]) {
        mvaddch(y, x, currentFrameBuffer[y][x]);
        lastFrameBuffer[y][x] = currentFrameBuffer[y][x];
      }
    }
  }

  wnoutrefresh(stdscr);
  doupdate();
  curs_set(0);
}

void Display::printPixel(Pixel pixel, bool isMoveableByCamera) {
  int printedX = pixel.getPosition().getX();
  int printedY = pixel.getPosition().getY();

  if (isMoveableByCamera) {
    printedX += currentCamera->getLengthOffset();
    printedY += currentCamera->getHeightOffset();
  }

  if (printedX >= 0 && printedX < SCREEN_LENGTH && printedY >= 0 &&
      printedY < SCREEN_HEIGHT) {
    currentFrameBuffer[printedY][printedX] = pixel.getCharacter();
  }
}

void Display::printSprite(Sprite sprite, bool isMoveableByCamera) {
  for (const Pixel &pixel : sprite.getPixels()) {
    printPixel(pixel, isMoveableByCamera);
  }
}

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
