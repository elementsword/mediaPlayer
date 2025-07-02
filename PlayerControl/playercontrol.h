#ifndef PLAYERSTATE_H
#define PLAYERSTATE_H
enum class PlayerState
{
    Playing,
    Paused,
    Quit
};
class PlayerControl
{
public:
    PlayerControl();
    ~PlayerControl();
    const PlayerState getState();
    void setState(const PlayerState &state);

private:
    PlayerState state;
};
#endif