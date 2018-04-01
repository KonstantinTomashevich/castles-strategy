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

void ProcessNewPlayerMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);
void ProcessPlayerTypeChangedMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);
void ProcessPlayerReadyChangedMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);
void ProcessPlayerLeftMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);

void ProcessUnitSpawnedMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);
void ProcessUnitsPullSyncMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);
void ProcessCoinsSyncMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);
void ProcessChatMessageMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);

NetworkManager::NetworkManager (IngameActivity *owner) : Urho3D::Object (owner->GetContext ()),
    owner_ (owner),
    incomingMessagesProcessors_ (STCNMT_TYPES_COUNT - STCNMT_START)
{
    SubscribeToEvent (Urho3D::E_NETWORKMESSAGE, URHO3D_HANDLER (NetworkManager, HandleNetworkMessage));
    incomingMessagesProcessors_ [STCNMT_GAME_STATUS - STCNMT_START] = ProcessGameStatusMessage;
    incomingMessagesProcessors_ [STCNMT_INITIAL_INFO - STCNMT_START] = ProcessInitialInfoMessage;

    incomingMessagesProcessors_ [STCNMT_NEW_PLAYER - STCNMT_START] = ProcessNewPlayerMessage;
    incomingMessagesProcessors_ [STCNMT_PLAYER_TYPE_CHANGED - STCNMT_START] = ProcessPlayerTypeChangedMessage;
    incomingMessagesProcessors_ [STCNMT_PLAYER_READY_CHANGED - STCNMT_START] = ProcessPlayerReadyChangedMessage;
    incomingMessagesProcessors_ [STCNMT_PLAYER_LEFT - STCNMT_START] = ProcessPlayerLeftMessage;

    incomingMessagesProcessors_ [STCNMT_UNIT_SPAWNED - STCNMT_START] = ProcessUnitSpawnedMessage;
    incomingMessagesProcessors_ [STCNMT_UNITS_PULL_SYNC - STCNMT_START] = ProcessUnitsPullSyncMessage;
    incomingMessagesProcessors_ [STCNMT_COINS_SYNC - STCNMT_START] = ProcessCoinsSyncMessage;
    incomingMessagesProcessors_ [STCNMT_CHAT_MESSAGE - STCNMT_START] = ProcessChatMessageMessage;
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

void NetworkManager::SendSpawnMessage (unsigned int spawnID, unsigned int unitType)
{
    Urho3D::VectorBuffer messageData;
    messageData.WriteUInt (spawnID);
    messageData.WriteUInt (unitType);

    Urho3D::Network *network = context_->GetSubsystem <Urho3D::Network> ();
    network->GetServerConnection ()->SendMessage (CTSNMT_SPAWN_UNIT, true, true, messageData);
}

void NetworkManager::SendChatMessage (const Urho3D::String &message)
{
    Urho3D::VectorBuffer messageData;
    messageData.WriteString (message);

    Urho3D::Network *network = context_->GetSubsystem <Urho3D::Network> ();
    network->GetServerConnection ()->SendMessage (CTSNMT_CHAT_MESSAGE, true, false, messageData);
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
    ingameActivity->SetGameStatus (static_cast <GameStatus> (messageData.ReadUInt ()));
}

void ProcessInitialInfoMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData)
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

    ingameActivity->GetFogOfWarManager ()->SetupFogOfWarMask (DEFAULT_FOG_OF_WAR_MASK_SIZE, mapXml.GetVector2 ("size"));
}

void ProcessNewPlayerMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData)
{
    Urho3D::String name = messageData.ReadString ();
    PlayerType playerType = static_cast <PlayerType> (messageData.ReadUByte ());
    bool readyForStart = messageData.ReadBool ();

    ingameActivity->GetDataManager ()->AddPlayer (name, playerType, readyForStart);
    if (name == ingameActivity->GetPlayerName ())
    {
        ingameActivity->SetPlayerType (playerType);
    }
}

void ProcessPlayerTypeChangedMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData)
{
    Urho3D::String name = messageData.ReadString ();
    PlayerType playerType = static_cast <PlayerType> (messageData.ReadUByte ());

    ingameActivity->GetDataManager ()->SetPlayerType (name, playerType);
    if (name == ingameActivity->GetPlayerName ())
    {
        ingameActivity->SetPlayerType (playerType);
    }
}

void ProcessPlayerReadyChangedMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData)
{
    Urho3D::String name = messageData.ReadString ();
    bool readyForStart = messageData.ReadBool ();
    ingameActivity->GetDataManager ()->SetIsPlayerReadyForStart (name, readyForStart);
}

void ProcessPlayerLeftMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData)
{
    Urho3D::String name = messageData.ReadString ();
    ingameActivity->GetDataManager ()->RemovePlayer (name);
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

void ProcessChatMessageMessage (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData)
{
    ingameActivity->GetIngameUIManager ()->AddNewChatMessage (messageData.ReadString ());
}
}
