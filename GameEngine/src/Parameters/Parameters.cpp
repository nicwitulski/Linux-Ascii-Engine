#include "../../include/Parameters.h"

std::vector<std::shared_ptr<Printable>> allPrintables;
bool printablesNeedSorted = true;

std::shared_ptr<Camera> currentCamera;

std::shared_ptr<Entity> playerEntity;

int SCREEN_HEIGHT = 24;
int SCREEN_LENGTH = 80;
int FRAME_RATE = 60;
int userInput = 0;
bool engineRunning = false;
bool displayNeedsCleared = false;