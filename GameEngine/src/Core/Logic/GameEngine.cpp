#include "../../../include/GameEngine.h"
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ncurses.h>
#include <thread>
#include <unistd.h>

using Clock = std::chrono::steady_clock;

void GameEngine::run() {
  userInput = 0;
  auto lastTime = Clock::now();
  const float FRAME_DURATION = 1.0f / FRAME_RATE;

  Display::initCurse();
  std::cout << "Made it here" << std::endl;
  engineRunning = true;
  while (engineRunning) {
    // User Input
    int ch;
    while ((ch = getch()) != ERR) {
      userInput = ch;
      if (userInput == '`') {
        engineRunning = false;
      }

      // State Update
      currentState->update();

      GameState *next = currentState->getNextState();
      if (next != nullptr) {
        currentState->onExit();
        delete currentState;
        currentState = next;
        currentState->onEnter();
      }
    }

    // --- FRAME UPDATES (LIMITED TO 60FPS) ---
    auto currentTime = Clock::now();
    std::chrono::duration<float> delta = currentTime - lastTime;
    if (delta.count() >= FRAME_DURATION) {

      // Display Refresh
      Display::refreshDisplay(delta.count());

      lastTime = currentTime;
    }

    // --- Tiny sleep to prevent CPU overuse ---
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  exit();
}

void GameEngine::exit() { Display::closeCurseWindow(); }