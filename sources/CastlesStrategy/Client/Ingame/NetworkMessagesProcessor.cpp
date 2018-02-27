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
    Urho3D::VectorBuffer messageData = data [Urho3D::NetworkMessage::P_DATA].GetVectorBuffer ();

    switch (messageID)
    {
        case STCNMT_GAME_STATUS: ProcessGameStatusMessage (messageData); break;
        case STCNMT_UNITS_TYPES_XML: ProcessUnitsTypesXMLMessage (messageData); break;
        default: break;
    }
}

void NetworkMessagesProcessor::ProcessGameStatusMessage (Urho3D::VectorBuffer &messageData)
{
    // TODO: Implement.
}

void NetworkMessagesProcessor::ProcessUnitsTypesXMLMessage (Urho3D::VectorBuffer &messageData)
{
    Urho3D::String xmlText = messageData.ReadString ();
    Urho3D::XMLFile *xmlFile = new Urho3D::XMLFile (context_);
    xmlFile->FromString (xmlText);

    owner_->GetDataProcessor ()->LoadUnitsTypesFromXML (xmlFile->GetRoot ());
    owner_->GetIngameUI ()->SetupUnitsIcons ();
}
}
