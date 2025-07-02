#include "playercontrol.h"
PlayerControl::PlayerControl() : state(PlayerState::Playing)
{
}
PlayerControl::~PlayerControl()
{
}
const PlayerState PlayerControl::getState()
{
    return state;
}
void PlayerControl::setState(const PlayerState &state)
{
    this->state = state;
}
