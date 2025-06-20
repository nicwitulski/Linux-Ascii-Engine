#include "StateLogic/States.h"

int main() {
  GameEngine engine(new StartAppState());
  engine.run();
  return 0;
}