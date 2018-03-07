#include "NetworkManager.hpp"
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Resource/ResourceCache.h>

#include <CastlesStrategy/Client/Ingame/IngameActivity.hpp>
#include <CastlesStrategy/Shared/Network/ServerConstants.hpp>
#include <CastlesStrategy/Shared/Network/ClientToServerNetworkMessageType.hpp>
#include <CastlesStrategy/Shared/Network/ServerToClientNetworkMessageType.hpp>

namespace CastlesStrategy
{
void ProcessGameStatusMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);
void ProcessMapPathMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);
void ProcessUnitSpawnedMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);

NetworkManager::NetworkManager (IngameActivity *owner) : Urho3D::Object (owner->GetContext ()),
    owner_ (owner),
    incomingMessagesProcessors_ (STCNMT_TYPES_COUNT - STCNMT_START)
{
    SubscribeToEvent (Urho3D::E_NETWORKMESSAGE, URHO3D_HANDLER (NetworkManager, HandleNetworkMessage));
    incomingMessagesProcessors_ [STCNMT_GAME_STATUS - STCNMT_START] = ProcessGameStatusMessage;
    incomingMessagesProcessors_ [STCNMT_MAP_PATH - STCNMT_START] = ProcessMapPathMessage;
    incomingMessagesProcessors_ [STCNMT_UNIT_SPAWNED - STCNMT_START] = ProcessUnitSpawnedMessage;
}

NetworkManager::~NetworkManager ()
{
    UnsubscribeFromAllEvents ();
}

void NetworkManager::HandleNetworkMessage (Urho3D::StringHash eventType, Urho3D::VariantMap &data)
{
    int messageID = data [Urho3D::NetworkMessage::P_MESSAGEID].GetInt ();
    Urho3D::VectorBuffer messageData = data [Urho3D::NetworkMessage::P_DATA].GetVectorBuffer ();
    incomingMessagesProcessors_ [messageID - STCNMT_START] (owner_, messageData);
}

void ProcessGameStatusMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData)
{
    // TODO: Implement.
}

void ProcessMapPathMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData)
{
    Urho3D::ResourceCache *resourceCache = ingameActivity->GetContext ()->GetSubsystem <Urho3D::ResourceCache> ();
    Urho3D::String mapPath = messageData.ReadString ();
    ingameActivity->GetScene ()->CreateChild ("PlayerSide", Urho3D::LOCAL)->LoadXML (
            resourceCache->GetResource <Urho3D::XMLFile> (mapPath + "PlayerSide.xml")->GetRoot ());

    Urho3D::XMLElement mapXml = resourceCache->GetResource <Urho3D::XMLFile> (mapPath + "Map.xml")->GetRoot ();
    Urho3D::XMLElement unitsXml = mapXml.GetBool ("useDefaultUnitsTypes") ?
            resourceCache->GetResource <Urho3D::XMLFile> (DEFAULT_UNITS_TYPES_PATH)->GetRoot () :
            resourceCache->GetResource <Urho3D::XMLFile> (mapPath + "UnitsTypes.xml")->GetRoot ();

    ingameActivity->GetDataManager ()->LoadUnitsTypesFromXML (unitsXml);
    ingameActivity->GetIngameUIManager ()->SetupUnitsIcons ();
    ingameActivity->GetCameraManager ()->SetupCamera (
            mapXml.GetVector3 ("defaultCameraPosition"), mapXml.GetQuaternion ("defaultCameraRotation"));
}

void ProcessUnitSpawnedMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData)
{
    ingameActivity->GetDataManager ()->AddPrefabToUnit (messageData.ReadUInt ());
}
}
