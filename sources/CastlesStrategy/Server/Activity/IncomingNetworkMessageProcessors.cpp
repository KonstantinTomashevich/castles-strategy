#include "IncomingNetworkMessageProcessors.hpp"
#include <Urho3D/IO/Log.h>

#include <CastlesStrategy/Server/Activity/ServerActivity.hpp>
#include <CastlesStrategy/Server/Managers/PlayersManager.hpp>
#include <CastlesStrategy/Server/Player/Player.hpp>

namespace CastlesStrategy
{
namespace IncomingNetworkMessageProcessors
{
void AddOrder (ServerActivity *activity, Urho3D::VectorBuffer &messageData, Urho3D::Connection *sender)
{
    if (sender == activity->GetFirstPlayer () || sender == activity->GetSecondPlayer ())
    {
        PlayersManager *playersManager = dynamic_cast <PlayersManager *> (
                activity->GetManagersHub ()->GetManager (MI_PLAYERS_MANAGER));

        Player &player = sender == activity->GetFirstPlayer () ?
                playersManager->GetFirstPlayer () : playersManager->GetSecondPlayer ();

        unsigned int unitType = messageData.ReadUInt ();
        player.AddOrder (unitType);
    }
}

void SpawnUnit (ServerActivity *activity, Urho3D::VectorBuffer &messageData, Urho3D::Connection *sender)
{
    // TODO: Implement.
}
}
}
