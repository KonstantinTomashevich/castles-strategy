#include "ServerActivity.hpp"
#include <CastlesStrategy/Shared/Network/ServerConstants.hpp>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Network/Network.h>

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Scene/SceneEvents.h>

#include <CastlesStrategy/Server/Managers/UnitsManager.hpp>
#include <CastlesStrategy/Server/Managers/PlayersManager.hpp>
#include <CastlesStrategy/Server/Managers/Map.hpp>
#include <CastlesStrategy/Server/Activity/IncomingNetworkMessageProcessors.hpp>

#include <CastlesStrategy/Shared/Network/ClientToServerNetworkMessageType.hpp>
#include <CastlesStrategy/Shared/Network/ServerToClientNetworkMessageType.hpp>
#include <CastlesStrategy/Shared/PlayerType.hpp>
#include <CastlesStrategy/Shared/ActivitiesControlEvents.hpp>
#include <Utils/UniversalException.hpp>

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
    incomingNetworkMessageProcessors_ (CTSNMT_TYPES_COUNT - CTSNMT_START),

    countOfPlayers_ (0),
    firstPlayer_ (nullptr),
    secondPlayer_ (nullptr)
{
    incomingNetworkMessageProcessors_ [CTSNMT_ADD_ORDER - CTSNMT_START] = IncomingNetworkMessageProcessors::AddOrder;
    incomingNetworkMessageProcessors_ [CTSNMT_SPAWN_UNIT - CTSNMT_START] = IncomingNetworkMessageProcessors::SpawnUnit;
    incomingNetworkMessageProcessors_ [CTSNMT_CHAT_MESSAGE - CTSNMT_START] =
            IncomingNetworkMessageProcessors::ChatMessage;

    incomingNetworkMessageProcessors_ [CTSNMT_REQUEST_TO_BE_A_PLAYER - CTSNMT_START] =
            IncomingNetworkMessageProcessors::RequestToBeAPlayer;

    incomingNetworkMessageProcessors_ [CTSNMT_REQUEST_TO_BE_AN_OBSERVER - CTSNMT_START] =
            IncomingNetworkMessageProcessors::RequestToBeAnObserver;

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

    // TODO: It's temporary. Autostarts game when 2 or more players are connected.
    else if (identifiedConnections_.Size () >= 2 && currentGameStatus_ == GS_WAITING)
    {
        managersHub_ = new ManagersHub (scene_);
        firstPlayer_ = identifiedConnections_.Front ().second_.connection_;
        secondPlayer_ = identifiedConnections_.Back ().second_.connection_;

        unsigned int startCoins;
        LoadResources (startCoins);
        SetupPlayers (startCoins);

        currentGameStatus_ = GS_PLAYING;
        ReportGameStatus (GS_PLAYING);
    }
}

void ServerActivity::Stop ()
{
    Urho3D::Network *network = context_->GetSubsystem <Urho3D::Network> ();
    network->StopServer ();
}

void ServerActivity::ProcessRequestToBeAPlayer (Urho3D::Connection *sender)
{
    if (countOfPlayers_ < 2)
    {
        for (auto &connectionData : identifiedConnections_)
        {
            if (connectionData.second_.connection_ == sender)
            {
                connectionData.second_.playerType = PT_REQUESTED_TO_BE_PLAYER;
                countOfPlayers_++;
                return;
            }
        }
    }
}

void ServerActivity::ProcessRequestToBeAnObserver (Urho3D::Connection *sender)
{
    if (countOfPlayers_ > 0)
    {
        for (auto &connectionData : identifiedConnections_)
        {
            if (connectionData.second_.connection_ == sender)
            {
                if (connectionData.second_.playerType == PT_OBSERVER)
                {
                    return;
                }

                connectionData.second_.playerType = PT_OBSERVER;
                countOfPlayers_--;
                return;
            }
        }
    }
}

void ServerActivity::SetIsPlayerReady (Urho3D::Connection *sender, bool isReady)
{
    for (auto &connectionData : identifiedConnections_)
    {
        if (connectionData.second_.connection_ == sender)
        {
            connectionData.second_.readyForStart_ = isReady;
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
}

const IdentifiedConnectionsMap &ServerActivity::GetIdentifiedConnections () const
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

    identifiedConnections_ [name] = {connection, PT_OBSERVER, false};
    Urho3D::VectorBuffer data;
    data.WriteInt (currentGameStatus_);
    connection->SendMessage (STCNMT_GAME_STATUS, true, false, data);
    connection->SetScene (scene_);
}

void ServerActivity::HandleClientDisconnected (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData)
{
    Urho3D::Connection *connection =
            dynamic_cast <Urho3D::Connection *> (eventData[Urho3D::ClientConnected::P_CONNECTION].GetPtr ());

    if (!RemoveUnidentifiedConnection (connection))
    {
        RemoveIdentifiedConnection (connection);
        if (connection == firstPlayer_)
        {
            ReportGameStatus (GS_SECOND_WON);
        }

        else if (connection == secondPlayer_)
        {
            ReportGameStatus (GS_FIRST_WON);
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

        if (dynamic_cast <Unit *> (eventData [Urho3D::ComponentAdded::P_COMPONENT].GetPtr ()) != nullptr)
        {
            Urho3D::Network *network = context_->GetSubsystem <Urho3D::Network> ();
            Urho3D::VectorBuffer messageData;
            messageData.WriteUInt (node->GetID ());

            for (auto &connectionData : identifiedConnections_)
            {
                connectionData.second_.connection_->SendMessage (STCNMT_UNIT_SPAWNED, true, false, messageData);
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
    ReportGameStatus (currentGameStatus_);
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

bool ServerActivity::RemoveIdentifiedConnection (Urho3D::Connection *connection)
{
    for (auto iterator = identifiedConnections_.Begin (); iterator != identifiedConnections_.End (); iterator++)
    {
        if (iterator->second_.connection_ == connection)
        {
            identifiedConnections_.Erase (iterator);
            return true;
        }
    }
    return false;
}

void ServerActivity::ReportGameStatus (GameStatus gameStatus) const
{
    Urho3D::VectorBuffer data;
    data.WriteInt (gameStatus);

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
    SendInitialInfoToPlayers (mapFolder);
}

void ServerActivity::SendInitialInfoToPlayers (const Urho3D::String &mapPath)
{
    for (auto &identifiedConnection : identifiedConnections_)
    {
        Urho3D::VectorBuffer messageBuffer;
        messageBuffer.WriteString (mapPath);
        identifiedConnection.second_.connection_->SendMessage (STCNMT_INITIAL_INFO, true, false, messageBuffer);
        SendPlayerTypeToPlayer (identifiedConnection);
    }
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

void ServerActivity::SendPlayerTypeToPlayer (const Urho3D::String &playerName)
{
    auto iterator = identifiedConnections_.Find (playerName);
    if (iterator != identifiedConnections_.End ())
    {
        SendPlayerTypeToPlayer (*iterator);
    }
    else
    {
        throw UniversalException <ServerActivity> (
                "ServerActivity: attempt to send player type of unidentified player!");
    }
}

void ServerActivity::SendPlayerTypeToPlayer (IdentifiedConnectionsMap::KeyValue &playerInfo)
{
    PlayerType playerType = playerInfo.second_.playerType;
    if (playerType == PT_REQUESTED_TO_BE_PLAYER)
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
    messageData.WriteUByte (playerType);
    playerInfo.second_.connection_->SendMessage (STCNMT_PLAYER_TYPE, true, false, messageData);
}
}

