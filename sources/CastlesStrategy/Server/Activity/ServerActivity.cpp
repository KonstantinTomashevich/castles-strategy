#include "ServerActivity.hpp"
#include <CastlesStrategy/Shared/Network/ServerConstants.hpp>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Network/Network.h>

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Scene/SceneEvents.h>

#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/IO/File.h>

#include <CastlesStrategy/Server/Managers/UnitsManager.hpp>
#include <CastlesStrategy/Server/Managers/PlayersManager.hpp>
#include <CastlesStrategy/Server/Managers/Map.hpp>
#include <CastlesStrategy/Server/Managers/VillagesManager.hpp>
#include <CastlesStrategy/Server/Activity/IncomingNetworkMessageProcessors.hpp>

#include <CastlesStrategy/Shared/Network/ClientToServerNetworkMessageType.hpp>
#include <CastlesStrategy/Shared/Network/ServerToClientNetworkMessageType.hpp>
#include <CastlesStrategy/Shared/PlayerType.hpp>
#include <CastlesStrategy/Shared/ActivitiesControlEvents.hpp>
#include <Utils/UniversalException.hpp>
#include <Urho3D/IO/FileSystem.h>

namespace CastlesStrategy
{
ServerActivity::ServerActivity (Urho3D::Context *context) : Activity (context),
    autoDisconnectTime_ (DEFAULT_AUTO_DISCONNECT_TIME),
    serverPort_ (DEFAULT_SERVER_PORT),

    currentGameStatus_ (GS_WAITING),
    unidentifiedConnections_ (),
    identifiedConnections_ (),

    managersHub_ (nullptr),
    scene_ (new Urho3D::Scene (context_)),
    mapName_ (),
    mapData_ (),

    incomingNetworkMessageProcessors_ (CTSNMT_TYPES_COUNT - CTSNMT_START),
    countOfPlayers_ (0),
    firstPlayer_ (nullptr),
    secondPlayer_ (nullptr)
{
    incomingNetworkMessageProcessors_ [CTSNMT_ADD_ORDER - CTSNMT_START] = IncomingNetworkMessageProcessors::AddOrder;
    incomingNetworkMessageProcessors_ [CTSNMT_SPAWN_UNIT - CTSNMT_START] = IncomingNetworkMessageProcessors::SpawnUnit;
    incomingNetworkMessageProcessors_ [CTSNMT_CHAT_MESSAGE - CTSNMT_START] =
            IncomingNetworkMessageProcessors::ChatMessage;

    incomingNetworkMessageProcessors_ [CTSNMT_REQUEST_TO_CHANGE_TYPE - CTSNMT_START] =
            IncomingNetworkMessageProcessors::RequestToChangeType;

    incomingNetworkMessageProcessors_ [CTSNMT_SET_IS_READY_FOR_START - CTSNMT_START] =
            IncomingNetworkMessageProcessors::SetIsReadyForStart;

    SubscribeToEvent (Urho3D::E_CLIENTCONNECTED, URHO3D_HANDLER (ServerActivity, HandleClientConnected));
    SubscribeToEvent (Urho3D::E_CLIENTIDENTITY, URHO3D_HANDLER (ServerActivity, HandleClientIdentity));
    SubscribeToEvent (Urho3D::E_CLIENTDISCONNECTED, URHO3D_HANDLER (ServerActivity, HandleClientDisconnected));
    SubscribeToEvent (Urho3D::E_NETWORKMESSAGE, URHO3D_HANDLER (ServerActivity, HandleNetworkMessage));

    SubscribeToEvent (Urho3D::E_COMPONENTADDED, URHO3D_HANDLER (ServerActivity, HandleComponentAdded));
    SubscribeToEvent (E_PLAYER_UNITS_PULL_SYNC, URHO3D_HANDLER (ServerActivity, HandlePlayerUnitsPullSync));
    SubscribeToEvent (E_PLAYER_COINS_SYNC, URHO3D_HANDLER (ServerActivity, HandlePlayerCoinsSync));
    SubscribeToEvent (E_GAME_ENDED, URHO3D_HANDLER (ServerActivity, HandleGameEnded));

    SubscribeToEvent (E_REQUEST_GAME_START, URHO3D_HANDLER (ServerActivity, HandleRequestGameStart));
    SubscribeToEvent (E_REQUEST_KICK_PLAYER, URHO3D_HANDLER (ServerActivity, HandleRequestKickPlayer));
    SubscribeToEvent (E_REQUEST_SELECT_MAP, URHO3D_HANDLER (ServerActivity, HandleRequestSelectMap));
}

ServerActivity::~ServerActivity ()
{
    if (managersHub_ != nullptr)
    {
        delete managersHub_;
    }

    scene_->Clear ();
    delete scene_;
}

void ServerActivity::Start ()
{
    Urho3D::Network *network = context_->GetSubsystem <Urho3D::Network> ();
    network->StartServer (serverPort_);
}

void ServerActivity::Update (float timeStep)
{
    ProcessUnidentifiedConnections (timeStep);
    if (managersHub_ != nullptr && currentGameStatus_ == GS_PLAYING)
    {
        managersHub_->HandleUpdate (timeStep);
    }
}

void ServerActivity::Stop ()
{
    Urho3D::Network *network = context_->GetSubsystem <Urho3D::Network> ();
    network->StopServer ();
}

void ServerActivity::ProcessRequestToChangeType (Urho3D::Connection *sender, PlayerType newType)
{
    if (newType != PT_REQUESTED_TO_BE_PLAYER && newType != PT_OBSERVER)
    {
        throw UniversalException <ServerActivity> (
                "ServerActivity: ProcessRequestToChangeType can be called only with observer or requested to be a player types!");
    }

    if (currentGameStatus_ != GS_WAITING)
    {
        throw UniversalException <ServerActivity> (
                "ServerActivity: ProcessRequestToChangeType can be called only while waiting for game start!");
    }

    if ((newType == PT_OBSERVER && countOfPlayers_ > 0) ||
            (newType == PT_REQUESTED_TO_BE_PLAYER && countOfPlayers_ < 2))
    {
        for (auto &connectionData : identifiedConnections_)
        {
            if (connectionData.second_.connection_ == sender)
            {
                if (connectionData.second_.playerType == newType)
                {
                    return;
                }

                connectionData.second_.playerType = newType;
                countOfPlayers_ += (newType == PT_OBSERVER ? -1 : 1);

                SendPlayerTypeToAllPlayers (connectionData);
                return;
            }
        }
    }
}

void ServerActivity::SetIsPlayerReady (Urho3D::Connection *sender, bool isReady)
{
    if (currentGameStatus_ != GS_WAITING)
    {
        throw UniversalException <ServerActivity> (
                "ServerActivity: SetIsPlayerReady can be called only while waiting for game start!");
    }

    for (auto &connectionData : identifiedConnections_)
    {
        if (connectionData.second_.connection_ == sender)
        {
            connectionData.second_.readyForStart_ = isReady;

            Urho3D::VectorBuffer messageData;
            messageData.WriteString (connectionData.first_);
            messageData.WriteBool (isReady);

            for (auto &anotherConnectionData : identifiedConnections_)
            {
                anotherConnectionData.second_.connection_->SendMessage (
                        STCNMT_PLAYER_READY_CHANGED, true, true, messageData);
            }
            return;
        }
    }
}

const Urho3D::String &ServerActivity::GetMapName () const
{
    return mapName_;
}

void ServerActivity::SetMapName (const Urho3D::String &mapName)
{
    mapName_ = mapName;
    CollectMapData ();

    for (auto &connectionData : identifiedConnections_)
    {
        connectionData.second_.connection_->SendMessage (STCNMT_MAP_FILES, true, false, mapData_);
    }
}

const ServerActivity::IdentifiedConnectionsMap &ServerActivity::GetIdentifiedConnections () const
{
    return identifiedConnections_;
}

ManagersHub *ServerActivity::GetManagersHub () const
{
    return managersHub_;
}

Urho3D::Connection *ServerActivity::GetFirstPlayer () const
{
    return firstPlayer_;
}

Urho3D::Connection *ServerActivity::GetSecondPlayer () const
{
    return secondPlayer_;
}

void ServerActivity::HandleClientConnected (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData)
{
    Urho3D::Connection *connection =
            dynamic_cast <Urho3D::Connection *> (eventData[Urho3D::ClientConnected::P_CONNECTION].GetPtr ());

    if (currentGameStatus_ == GS_WAITING)
    {
        unidentifiedConnections_.Push (Urho3D::MakePair (connection, autoDisconnectTime_));
    }
    else
    {
        connection->Disconnect ();
    }
}

void ServerActivity::HandleClientIdentity (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData)
{
    Urho3D::Connection *connection =
            dynamic_cast <Urho3D::Connection *> (eventData[Urho3D::ClientConnected::P_CONNECTION].GetPtr ());
    Urho3D::String name = eventData [IdentityFields::NAME].GetString ();

    RemoveUnidentifiedConnection (connection);
    if (identifiedConnections_.Contains (name) || currentGameStatus_ != GS_WAITING)
    {
        connection->Disconnect ();
        return;
    }

    Urho3D::VectorBuffer newPlayerMessageData;
    newPlayerMessageData.WriteString (name);
    newPlayerMessageData.WriteUByte (PT_OBSERVER);
    newPlayerMessageData.WriteBool (false);
    connection->SendMessage (STCNMT_NEW_PLAYER, true, true, newPlayerMessageData);

    for (auto &anotherConnectionData : identifiedConnections_)
    {
        anotherConnectionData.second_.connection_->SendMessage (STCNMT_NEW_PLAYER, true, true, newPlayerMessageData);

        Urho3D::VectorBuffer messageData;
        messageData.WriteString (anotherConnectionData.first_);
        messageData.WriteUByte (anotherConnectionData.second_.playerType);
        messageData.WriteBool (anotherConnectionData.second_.readyForStart_);

        connection->SendMessage (STCNMT_NEW_PLAYER, true, true, messageData);
    }

    identifiedConnections_ [name] = {connection, PT_OBSERVER, false};
    Urho3D::VectorBuffer data;
    data.WriteInt (currentGameStatus_);

    connection->SendMessage (STCNMT_GAME_STATUS, true, false, data);
    connection->SetScene (scene_);
    connection->SendMessage (STCNMT_MAP_FILES, true, false, mapData_);
}

void ServerActivity::HandleClientDisconnected (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData)
{
    Urho3D::Connection *connection =
            dynamic_cast <Urho3D::Connection *> (eventData[Urho3D::ClientConnected::P_CONNECTION].GetPtr ());

    if (!RemoveUnidentifiedConnection (connection))
    {
        Urho3D::VectorBuffer messageData;
        messageData.WriteString (RemoveIdentifiedConnection (connection));

        for (auto &identifiedConnectionData : identifiedConnections_)
        {
            identifiedConnectionData.second_.connection_->SendMessage (STCNMT_PLAYER_LEFT, true, false, messageData);
        }

        if (currentGameStatus_ == GS_PLAYING)
        {
            if (connection == firstPlayer_)
            {
                currentGameStatus_ = GS_SECOND_WON;
                ReportGameStatus ();
            }

            else if (connection == secondPlayer_)
            {
                currentGameStatus_ = GS_FIRST_WON;
                ReportGameStatus ();
            }
        }
    }
}

void ServerActivity::HandleNetworkMessage (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData)
{
    int messageId = eventData [Urho3D::NetworkMessage::P_MESSAGEID].GetInt ();
    if (messageId >= CTSNMT_START && messageId < CTSNMT_TYPES_COUNT)
    {
        Urho3D::VectorBuffer messageData = eventData [Urho3D::NetworkMessage::P_DATA].GetVectorBuffer ();
        incomingNetworkMessageProcessors_[messageId - CTSNMT_START] (this, messageData,
                dynamic_cast <Urho3D::Connection *> (eventData[Urho3D::NetworkMessage::P_CONNECTION].GetPtr ()));
    }
}

void ServerActivity::HandleComponentAdded (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData)
{
    Urho3D::Scene *scene = static_cast <Urho3D::Scene *> (eventData [Urho3D::ComponentAdded::P_SCENE].GetVoidPtr ());
    if (scene == scene_)
    {
        Urho3D::Node *node = static_cast <Urho3D::Node *> (eventData [Urho3D::ComponentAdded::P_NODE].GetVoidPtr ());
        Urho3D::RefCounted *component = eventData [Urho3D::ComponentAdded::P_COMPONENT].GetPtr ();

        if (dynamic_cast <Unit *> (component) != nullptr || dynamic_cast <Village *> (component) != nullptr)
        {
            Urho3D::Network *network = context_->GetSubsystem <Urho3D::Network> ();
            Urho3D::VectorBuffer messageData;
            messageData.WriteUInt (node->GetID ());

            for (auto &connectionData : identifiedConnections_)
            {
                connectionData.second_.connection_->SendMessage (STCNMT_OBJECT_SPAWNED, true, false, messageData);
            }
        }
    }
}

void ServerActivity::HandlePlayerUnitsPullSync (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData)
{
    Player *player = static_cast <Player *> (eventData [PlayerUnitsPullSync::PLAYER].GetVoidPtr ());
    Urho3D::VectorBuffer messageData;
    messageData.WriteUInt (eventData [PlayerUnitsPullSync::UNIT_TYPE].GetUInt ());
    messageData.WriteUInt (eventData [PlayerUnitsPullSync::NEW_VALUE].GetUInt ());

    (player == &dynamic_cast <PlayersManager *> (managersHub_->GetManager (MI_PLAYERS_MANAGER))->GetFirstPlayer () ?
        firstPlayer_ : secondPlayer_)->SendMessage (STCNMT_UNITS_PULL_SYNC, true, false, messageData);
}

void ServerActivity::HandlePlayerCoinsSync (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData)
{
    Player *player = static_cast <Player *> (eventData [PlayerUnitsPullSync::PLAYER].GetVoidPtr ());
    Urho3D::VectorBuffer messageData;
    messageData.WriteUInt (eventData [PlayerCoinsSync::NEW_VALUE].GetUInt ());

    (player == &dynamic_cast <PlayersManager *> (managersHub_->GetManager (MI_PLAYERS_MANAGER))->GetFirstPlayer () ?
            firstPlayer_ : secondPlayer_)->SendMessage (STCNMT_COINS_SYNC, true, false, messageData);
}

void ServerActivity::HandleGameEnded (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData)
{
    currentGameStatus_ = eventData [GameEnded::FIRST_WON].GetBool () ? GS_FIRST_WON : GS_SECOND_WON;
    ReportGameStatus ();
}

void ServerActivity::HandleRequestGameStart (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData)
{
    if (countOfPlayers_ != 2 || currentGameStatus_ != GS_WAITING)
    {
        return;
    }

    IdentifiedConnectionsMap::KeyValue *firstData = nullptr;
    IdentifiedConnectionsMap::KeyValue *secondData = nullptr;

    for (auto &connection : identifiedConnections_)
    {
        if (!connection.second_.readyForStart_)
        {
            return;
        }

        if (connection.second_.playerType == PT_REQUESTED_TO_BE_PLAYER)
        {
            if (firstData == nullptr)
            {
                firstData = &connection;
            }
            else if (secondData == nullptr)
            {
                secondData = &connection;
            }
            else
            {
                throw UniversalException <ServerActivity> (
                        "ServerActivity: more than 2 connections requested to be players!");
            }
        }
    }

    if (firstData == nullptr || secondData == nullptr)
    {
        throw UniversalException <ServerActivity> (
                "ServerActivity: count of players is two, but less players found!");
    }

    managersHub_ = new ManagersHub (scene_);
    firstPlayer_ = firstData->second_.connection_;
    secondPlayer_ = secondData->second_.connection_;

    currentGameStatus_ = GS_PLAYING;
    unsigned int startCoins;
    LoadResources (startCoins);
    SetupPlayers (startCoins);
    ReportGameStatus ();

    SendPlayerTypeToAllPlayers (*firstData);
    SendPlayerTypeToAllPlayers (*secondData);
}

void ServerActivity::HandleRequestKickPlayer (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData)
{
    IdentifiedConnectionsMap::Iterator iterator =
            identifiedConnections_.Find (eventData [RequestKickPlayer::PLAYER_NAME].GetString ());

    if (iterator != identifiedConnections_.End () &&
            (iterator->second_.playerType == PT_OBSERVER || currentGameStatus_ == GS_WAITING))
    {
        iterator->second_.connection_->Disconnect ();
    }
}

void ServerActivity::HandleRequestSelectMap (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData)
{
    Urho3D::String mapName = eventData [RequestSelectMap::MAP_NAME].GetString ();
    if (!context_->GetSubsystem <Urho3D::FileSystem> ()->FileExists (
            "Data/" + DEFAULT_MAPS_FOLDER + "/" + mapName + "/Map.xml"))
    {
        throw UniversalException <ServerActivity> (
                "ServerActivity: map \"" + mapName + "\" is not exists!");
    }
    SetMapName (mapName);
}

bool ServerActivity::RemoveUnidentifiedConnection (Urho3D::Connection *connection)
{
    for (auto iterator = unidentifiedConnections_.Begin (); iterator != unidentifiedConnections_.End (); iterator++)
    {
        if (iterator->first_ == connection)
        {
            unidentifiedConnections_.Erase (iterator);
            return true;
        }
    }
    return false;
}

Urho3D::String ServerActivity::RemoveIdentifiedConnection (Urho3D::Connection *connection)
{
    for (auto iterator = identifiedConnections_.Begin (); iterator != identifiedConnections_.End (); iterator++)
    {
        if (iterator->second_.connection_ == connection)
        {
            if (iterator->second_.playerType == PT_REQUESTED_TO_BE_PLAYER)
            {
                countOfPlayers_--;
            }

            Urho3D::String name = iterator->first_;
            identifiedConnections_.Erase (iterator);
            return name;
        }
    }
    return Urho3D::String::EMPTY;
}

void ServerActivity::ReportGameStatus () const
{
    Urho3D::VectorBuffer data;
    data.WriteInt (currentGameStatus_);

    for (auto &connectionData : identifiedConnections_)
    {
        connectionData.second_.connection_->SendMessage (STCNMT_GAME_STATUS, true, false, data);
    }
}

void ServerActivity::ProcessUnidentifiedConnections (float timeStep)
{
    for (auto iterator = unidentifiedConnections_.Begin (); iterator != unidentifiedConnections_.End ();)
    {
        iterator->second_ -= timeStep;
        if (iterator->second_ <= 0.0f)
        {
            iterator->first_->Disconnect ();
            iterator = unidentifiedConnections_.Erase (iterator);
        }
        else
        {
            iterator++;
        }
    }
}

void ServerActivity::LoadResources (unsigned int &startCoins)
{
    Urho3D::String mapFolder = DEFAULT_MAPS_FOLDER + Urho3D::String ("/") + mapName_ + "/";
    bool useDefaultUnitsTypes;

    LoadScene (mapFolder);
    LoadMap (mapFolder, startCoins, useDefaultUnitsTypes);
    LoadUnitsTypesAndSpawns (mapFolder, useDefaultUnitsTypes);
    LoadVillages (mapFolder);
}

void ServerActivity::LoadScene (const Urho3D::String &mapFolder)
{
    Urho3D::ResourceCache *resourceCache = context_->GetSubsystem <Urho3D::ResourceCache> ();
    Urho3D::XMLFile *sceneXMLFile = resourceCache->GetResource <Urho3D::XMLFile> (mapFolder + "Scene.xml");
    if (sceneXMLFile == nullptr)
    {
        throw UniversalException <ServerActivity> ("ServerActivity: can not find scene xml!");
    }

    if (!scene_->LoadXML (sceneXMLFile->GetRoot ()))
    {
        throw UniversalException <ServerActivity> ("ServerActivity: can not load scene from xml!");
    }
}

void ServerActivity::LoadMap (const Urho3D::String &mapFolder, unsigned int &startCoins, bool &useDefaultUnitsTypes)
{
    Urho3D::ResourceCache *resourceCache = context_->GetSubsystem <Urho3D::ResourceCache> ();
    Urho3D::XMLFile *mapXMLFile = resourceCache->GetResource <Urho3D::XMLFile> (mapFolder + "Map.xml");

    if (mapXMLFile == nullptr)
    {
        throw UniversalException <ServerActivity> ("ServerActivity: can not find map " + mapName_ + " xml!");
    }

    Urho3D::XMLElement mapXML = mapXMLFile->GetRoot ();
    useDefaultUnitsTypes = mapXML.GetBool ("useDefaultUnitsTypes");
    startCoins = mapXML.GetUInt ("startCoins");

    Map *map = dynamic_cast <Map *> (managersHub_->GetManager (MI_MAP));
    map->SetSize (mapXML.GetIntVector2 ("size"));
    map->LoadRoutesFromXML (mapXML);
}

void ServerActivity::LoadUnitsTypesAndSpawns (const Urho3D::String &mapFolder, bool useDefaultUnitsTypes)
{
    Urho3D::ResourceCache *resourceCache = context_->GetSubsystem <Urho3D::ResourceCache> ();
    UnitsManager *unitsManager = dynamic_cast <UnitsManager *> (managersHub_->GetManager (MI_UNITS_MANAGER));

    Urho3D::String unitsTypesXMLPath = useDefaultUnitsTypes ? DEFAULT_UNITS_TYPES_PATH : mapFolder + "UnitsTypes.xml";
    Urho3D::XMLFile *unitsTypesXMLFile = resourceCache->GetResource <Urho3D::XMLFile> (unitsTypesXMLPath);

    if (unitsTypesXMLFile == nullptr)
    {
        throw UniversalException <ServerActivity> (
                "ServerActivity: can not find units types xml " + unitsTypesXMLPath + "!");
    }

    unitsManager->LoadUnitsTypesFromXML (unitsTypesXMLFile->GetRoot ());
    unitsManager->LoadSpawnsFromXML (resourceCache->GetResource <Urho3D::XMLFile> (mapFolder + "Map.xml")->GetRoot ());
}

void ServerActivity::LoadVillages (const Urho3D::String &mapFolder)
{
    Urho3D::ResourceCache *resourceCache = context_->GetSubsystem <Urho3D::ResourceCache> ();
    VillagesManager *villagesManager = dynamic_cast <VillagesManager *> (managersHub_->GetManager (MI_VILLAGES_MANAGER));
    villagesManager->LoadVillagesFromXML (
            resourceCache->GetResource <Urho3D::XMLFile> (mapFolder + "Map.xml")->GetRoot ());
}

void ServerActivity::SetupPlayers (unsigned int startCoins)
{
    PlayersManager *playersManager = dynamic_cast <PlayersManager *> (managersHub_->GetManager (MI_PLAYERS_MANAGER));
    Player firstPlayer (managersHub_);
    playersManager->SetFirstPlayer (firstPlayer);
    playersManager->GetFirstPlayer ().SetCoins (startCoins);

    Player secondPlayer (managersHub_);
    playersManager->SetSecondPlayer (secondPlayer);
    playersManager->GetSecondPlayer ().SetCoins (startCoins);
}

void ServerActivity::SendPlayerTypeToAllPlayers (const Urho3D::String &playerName)
{
    auto iterator = identifiedConnections_.Find (playerName);
    if (iterator != identifiedConnections_.End ())
    {
        SendPlayerTypeToAllPlayers (*iterator);
    }
    else
    {
        throw UniversalException <ServerActivity> (
                "ServerActivity: attempt to send player type of unidentified player!");
    }
}

void ServerActivity::SendPlayerTypeToAllPlayers (IdentifiedConnectionsMap::KeyValue &playerInfo)
{
    PlayerType playerType = playerInfo.second_.playerType;
    if (playerType == PT_REQUESTED_TO_BE_PLAYER && currentGameStatus_ != GS_WAITING)
    {
        if (firstPlayer_ == playerInfo.second_.connection_)
        {
            playerType = PT_FIRST;
        }
        else if (secondPlayer_ == playerInfo.second_.connection_)
        {
            playerType = PT_SECOND;
        }
        else
        {
            throw UniversalException <ServerActivity> (
                    "ServerActivity: more than 2 connections requested to be players!");
        }

        playerInfo.second_.playerType = playerType;
    }

    Urho3D::VectorBuffer messageData;
    messageData.WriteString (playerInfo.first_);
    messageData.WriteUByte (playerType);

    for (auto &connection : identifiedConnections_)
    {
        connection.second_.connection_->SendMessage (STCNMT_PLAYER_TYPE_CHANGED, true, true, messageData);
    }
}

void ServerActivity::CollectMapData ()
{
    mapData_.Clear ();
    Urho3D::Vector <Urho3D::String> mapFiles;
    context_->GetSubsystem <Urho3D::FileSystem> ()->ScanDir (
            mapFiles, "Data/" + DEFAULT_MAPS_FOLDER + "/" + mapName_, "*", Urho3D::SCAN_FILES, true);

    mapData_.WriteString (mapName_);
    mapData_.WriteUInt (mapFiles.Size ());

    for (auto &fileName : mapFiles)
    {
        Urho3D::File *file = new Urho3D::File (context_,
                "Data/" + DEFAULT_MAPS_FOLDER + "/" + mapName_ + "/" + fileName, Urho3D::FILE_READ);

        Urho3D::PODVector <unsigned char> buffer (file->GetSize ());
        file->Read (&buffer [0], file->GetSize ());
        file->Close ();

        mapData_.WriteString (fileName);
        mapData_.WriteBuffer (buffer);
    }
}
}

