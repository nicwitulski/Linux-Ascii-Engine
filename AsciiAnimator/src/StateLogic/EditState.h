//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file EditState.h
/// @author (your name)
/// @brief Animation editor state for AsciiAnimator
/// @version 0.1
/// @date 2025-07-02
///
/// @copyright Copyright (c) 2025
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef EDITSTATE_H
#define EDITSTATE_H

#include "../../../GameEngine/include/GameEngine.h"
#include <string>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class EditState
///
/// State for editing an animation (drawing, color picking, frame navigation, etc.)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class EditState : public GameState
{
private:
    std::string printableName;
    std::string animationName;
    // TODO: Add members for animation, frame, cursor, colors, etc.

public:
    EditState(const std::string& printable, const std::string& animation);
    void onEnter() override;
    void update() override;
    void onExit() override;
    GameState* getNextState() override;
};

#endif 