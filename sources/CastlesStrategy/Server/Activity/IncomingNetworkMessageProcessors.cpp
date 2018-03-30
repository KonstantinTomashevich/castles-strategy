#include "IncomingNetworkMessageProcessors.hpp"
#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Network/Network.h>

#include <CastlesStrategy/Server/Activity/ServerActivity.hpp>
#include <CastlesStrategy/Server/Managers/PlayersManager.hpp>
#include <CastlesStrategy/Server/Managers/UnitsManager.hpp>
#include <CastlesStrategy/Server/Player/Player.hpp>

#include <CastlesStrategy/Shared/Network/ServerToClientNetworkMessageType.hpp>
#include <Utils/UniversalException.hpp>

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
    if (sender == activity->GetFirstPlayer () || sender == activity->GetSecondPlayer ())
    {
        unsigned int spawnID = messageData.ReadUInt ();
        unsigned int unitType = messageData.ReadUInt ();

        PlayersManager *playersManager = dynamic_cast <PlayersManager *> (
                activity->GetManagersHub ()->GetManager (MI_PLAYERS_MANAGER));

        Player &player = sender == activity->GetFirstPlayer () ?
                playersManager->GetFirstPlayer () : playersManager->GetSecondPlayer ();
        player.TakeUnitFromPull (unitType);

        UnitsManager *unitsManager = dynamic_cast <UnitsManager *> (
                activity->GetManagersHub ()->GetManager (MI_UNITS_MANAGER));
        unitsManager->SpawnUnit (spawnID, unitType);
    }
}

void ChatMessage (ServerActivity *activity, Urho3D::VectorBuffer &messageData, Urho3D::Connection *sender)
{
    Urho3D::String resultingMessage;
    resultingMessage += Urho3D::Time::GetTimeStamp ().Substring (11, 8);

    bool found = false;
    for (const auto &item : activity->GetIdentifiedConnections ())
    {
        if (item.second_.connection_ == sender)
        {
            resultingMessage += " [" + item.first_ + "] ";
            found = true;
            break;
        }
    }

    if (!found)
    {
        URHO3D_LOGERROR ("ServerActivity: unidentified player attempted to send a chat message!");
    }

    resultingMessage += messageData.ReadString ();
    Urho3D::VectorBuffer newMessageData;
    newMessageData.WriteString (resultingMessage);

    for (const auto &item : activity->GetIdentifiedConnections ())
    {
        item.second_.connection_->SendMessage (STCNMT_CHAT_MESSAGE, true, false, newMessageData);
    }
}
}
}
