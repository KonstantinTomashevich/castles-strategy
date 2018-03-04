#include "NetworkMessagesProcessor.hpp"
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Resource/ResourceCache.h>

#include <CastlesStrategy/Client/Ingame/IngameActivity.hpp>
#include <CastlesStrategy/Shared/Network/ServerConstants.hpp>
#include <CastlesStrategy/Shared/Network/ClientToServerNetworkMessageType.hpp>
#include <CastlesStrategy/Shared/Network/ServerToClientNetworkMessageType.hpp>

namespace CastlesStrategy
{
NetworkMessagesProcessor::NetworkMessagesProcessor (IngameActivity *owner) : Urho3D::Object (owner->GetContext ()),
    owner_ (owner)
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

    // TODO: Use vector of processors instead of switching!
    switch (messageID)
    {
        case STCNMT_GAME_STATUS: ProcessGameStatusMessage (messageData); break;
        case STCNMT_MAP_PATH: ProcessMapPathMessage (messageData); break;
        default: break;
    }
}

void NetworkMessagesProcessor::ProcessGameStatusMessage (Urho3D::VectorBuffer &messageData)
{
    // TODO: Implement.
}

void NetworkMessagesProcessor::ProcessMapPathMessage (Urho3D::VectorBuffer &messageData)
{
    Urho3D::ResourceCache *resourceCache = context_->GetSubsystem <Urho3D::ResourceCache> ();
    Urho3D::String mapPath = messageData.ReadString ();
    owner_->GetScene ()->CreateChild ("PlayerSide", Urho3D::LOCAL)->LoadXML (
            resourceCache->GetResource <Urho3D::XMLFile> (mapPath + "PlayerSide.xml")->GetRoot ());

    Urho3D::XMLElement mapXml = resourceCache->GetResource <Urho3D::XMLFile> (mapPath + "Map.xml")->GetRoot ();
    Urho3D::XMLElement unitsXml = mapXml.GetBool ("useDefaultUnitsTypes") ?
            resourceCache->GetResource <Urho3D::XMLFile> (DEFAULT_UNITS_TYPES_PATH)->GetRoot () :
            resourceCache->GetResource <Urho3D::XMLFile> (mapPath + "UnitsTypes.xml")->GetRoot ();

    owner_->GetDataProcessor ()->LoadUnitsTypesFromXML (unitsXml);
    owner_->GetIngameUI ()->SetupUnitsIcons ();
}
}
