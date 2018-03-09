#include "NetworkManager.hpp"
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Resource/ResourceCache.h>

#include <CastlesStrategy/Client/Ingame/IngameActivity.hpp>
#include <CastlesStrategy/Shared/Network/ServerConstants.hpp>
#include <CastlesStrategy/Shared/Network/ClientToServerNetworkMessageType.hpp>
#include <CastlesStrategy/Shared/Network/ServerToClientNetworkMessageType.hpp>

namespace CastlesStrategy
{
void ProcessGameStatusMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);
void ProcessInitialInfoMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);
void ProcessUnitSpawnedMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);
void ProcessUnitsPullSyncMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);
void ProcessCoinsSyncMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);

NetworkManager::NetworkManager (IngameActivity *owner) : Urho3D::Object (owner->GetContext ()),
    owner_ (owner),
    incomingMessagesProcessors_ (STCNMT_TYPES_COUNT - STCNMT_START)
{
    SubscribeToEvent (Urho3D::E_NETWORKMESSAGE, URHO3D_HANDLER (NetworkManager, HandleNetworkMessage));
    incomingMessagesProcessors_ [STCNMT_GAME_STATUS - STCNMT_START] = ProcessGameStatusMessage;
    incomingMessagesProcessors_ [STCNMT_INITIAL_INFO - STCNMT_START] = ProcessInitialInfoMessage;
    incomingMessagesProcessors_ [STCNMT_UNIT_SPAWNED - STCNMT_START] = ProcessUnitSpawnedMessage;
    incomingMessagesProcessors_ [STCNMT_UNITS_PULL_SYNC - STCNMT_START] = ProcessUnitsPullSyncMessage;
    incomingMessagesProcessors_ [STCNMT_COINS_SYNC - STCNMT_START] = ProcessCoinsSyncMessage;
}

NetworkManager::~NetworkManager ()
{
    UnsubscribeFromAllEvents ();
}

void NetworkManager::SendAddOrderMessage (unsigned int unitType) const
{
    Urho3D::VectorBuffer messageData;
    messageData.WriteUInt (unitType);

    Urho3D::Network *network = context_->GetSubsystem <Urho3D::Network> ();
    network->GetServerConnection ()->SendMessage (CTSNMT_ADD_ORDER, true, true, messageData);
}

void NetworkManager::HandleNetworkMessage (Urho3D::StringHash eventType, Urho3D::VariantMap &data)
{
    int messageID = data [Urho3D::NetworkMessage::P_MESSAGEID].GetInt ();
    if (messageID >= STCNMT_START && messageID < STCNMT_TYPES_COUNT)
    {
        Urho3D::VectorBuffer messageData = data[Urho3D::NetworkMessage::P_DATA].GetVectorBuffer ();
        incomingMessagesProcessors_[messageID - STCNMT_START] (owner_, messageData);
    }
}

void ProcessGameStatusMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData)
{
    // TODO: Implement.
}

void ProcessInitialInfoMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData)
{
    PlayerType playerType = static_cast <PlayerType> (messageData.ReadUByte ());
    ingameActivity->SetPlayerType (playerType);

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

void ProcessUnitsPullSyncMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData)
{
    unsigned int unitType = messageData.ReadUInt ();
    unsigned int newValue = messageData.ReadUInt ();
    ingameActivity->GetDataManager ()->UpdateUnitsPull (unitType, newValue);
}

void ProcessCoinsSyncMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData)
{
    ingameActivity->GetDataManager ()->SetPredictedCoins (messageData.ReadUInt ());
}
}
