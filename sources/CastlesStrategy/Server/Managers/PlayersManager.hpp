#pragma once
#include <unordered_map>
#include <CastlesStrategy/Server/Managers/Manager.hpp>
#include <CastlesStrategy/Server/Player/Player.hpp>

namespace CastlesStrategy
{
class PlayersManager : public Manager
{
public:
    explicit PlayersManager (ManagersHub *managersHub);
    virtual ~PlayersManager ();

    virtual void HandleUpdate (float timeStep);
    Player &GetFirstPlayer ();
    const Player &GetFirstPlayer () const;
    void SetFirstPlayer (const Player &firstPlayer);

    Player &GetSecondPlayer ();
    const Player &GetSecondPlayer () const;
    void SetSecondPlayer (const Player &secondPlayer);

private:
    Player firstPlayer_;
    Player secondPlayer_;
};
}
