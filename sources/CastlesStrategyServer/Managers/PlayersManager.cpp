#include "PlayersManager.hpp"

namespace CastlesStrategy
{
PlayersManager::PlayersManager (ManagersHub *managersHub) : Manager (managersHub),
    firstPlayer_ (managersHub),
    secondPlayer_ (managersHub)
{

}

PlayersManager::~PlayersManager ()
{

}

void PlayersManager::HandleUpdate (float timeStep)
{
    firstPlayer_.HandleUpdate (timeStep);
    secondPlayer_.HandleUpdate (timeStep);
}

Player &PlayersManager::GetFirstPlayer ()
{
    return firstPlayer_;
}

const Player &PlayersManager::GetFirstPlayer () const
{
    return firstPlayer_;
}

void PlayersManager::SetFirstPlayer (const Player &firstPlayer)
{
    firstPlayer_ = firstPlayer;
}

Player &PlayersManager::GetSecondPlayer ()
{
    return secondPlayer_;
}

const Player &PlayersManager::GetSecondPlayer () const
{
    return secondPlayer_;
}

void PlayersManager::SetSecondPlayer (const Player &secondPlayer)
{
    secondPlayer_ = secondPlayer;
}
}
