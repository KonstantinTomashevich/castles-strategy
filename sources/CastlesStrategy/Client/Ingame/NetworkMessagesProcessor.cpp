#include "NetworkMessagesProcessor.hpp"
#include <Urho3D/Network/NetworkEvents.h>

#include <CastlesStrategy/Client/Ingame/IngameActivity.hpp>
#include <CastlesStrategy/Shared/Network/ClientToServerNetworkMessageType.hpp>
#include <CastlesStrategy/Shared/Network/ServerToClientNetworkMessageType.hpp>

namespace CastlesStrategy
{
NetworkMessagesProcessor::NetworkMessagesProcessor (IngameActivity *owner) : Urho3D::Object (owner->GetContext ())
{
    SubscribeToEvent (Urho3D::E_NETWORKMESSAGE, URHO3D_HANDLER (NetworkMessagesProcessor, HandleNetworkMessage));
}

NetworkMessagesProcessor::~NetworkMessagesProcessor ()
{
    UnsubscribeFromAllEvents ();
}

void NetworkMessagesProcessor::HandleNetworkMessage (Urho3D::StringHash eventType, Urho3D::VariantMap &data)
{
    int messageID = data [Urho3D::NetworkMessage::P_MESSAGEID].GetInt ();
    const Urho3D::VariantMap &messageData = data [Urho3D::NetworkMessage::P_DATA].GetVariantMap ();

    switch (messageID)
    {
        case STCNMT_GAME_STATUS: ProcessGameStatusMessage (messageData); break;
        case STCNMT_UNITS_TYPES_XML: ProcessUnitsTypesXMLMessage (messageData); break;
        default: break;
    }
}

void NetworkMessagesProcessor::ProcessGameStatusMessage (const Urho3D::VariantMap &messageData)
{
    // TODO: Implement.
}

void NetworkMessagesProcessor::ProcessUnitsTypesXMLMessage (const Urho3D::VariantMap &messageData)
{
    // TODO: Implement.
}
}
