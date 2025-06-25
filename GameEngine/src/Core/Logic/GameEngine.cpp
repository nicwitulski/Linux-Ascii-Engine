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

  Display::initCurse();
  engineRunning = true;

  while (engineRunning) {
    // --- Time Tracking ---
    auto currentTime = Clock::now();
    std::chrono::duration<float> delta = currentTime - lastTime;
    float deltaTime = delta.count();
    lastTime = currentTime;

    // --- Input Handling ---
    int ch;
    while ((ch = getch()) != ERR) {
      userInput = ch;
      if (userInput == '`')
        engineRunning = false;
    }
    // State Update
    currentState->update();
    GameState *next = currentState->getNextState();
    if (next) {
      currentState->onExit();
      delete currentState;
      currentState = next;
      currentState->onEnter();
    }

    // --- Refresh display using actual deltaTime ---
    Display::refreshDisplay(deltaTime);

    std::this_thread::sleep_for(
        std::chrono::milliseconds(1)); // Prevent CPU maxing out
  }

  exit();
}

void GameEngine::exit() { Display::closeCurseWindow(); }