#include "ServerActivity.hpp"
#include <CastlesStrategy/Shared/Network/ServerConstants.hpp>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Network/NetworkEvents.h>
#include <Urho3D/Network/Network.h>

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>

#include <CastlesStrategy/Server/Managers/UnitsManager.hpp>
#include <CastlesStrategy/Server/Managers/PlayersManager.hpp>
#include <CastlesStrategy/Server/Managers/Map.hpp>

#include <CastlesStrategy/Shared/Network/ClientToServerNetworkMessageType.hpp>
#include <CastlesStrategy/Shared/Network/ServerToClientNetworkMessageType.hpp>
#include <CastlesStrategy/Server/Activity/IncomingNetworkMessageProcessors.hpp>
#include <Utils/UniversalException.hpp>

namespace CastlesStrategy
{
ServerActivity::ServerActivity (Urho3D::Context *context) : Activity (context),
    currentGameStatus_ (GS_WAITING),
    unidentifiedConnections_ (),
    identifiedConnections_ (),

    managersHub_ (nullptr),
    scene_ (new Urho3D::Scene (context_)),
    mapName_ (),
    incomingNetworkMessageProcessors_ (CTSNMT_TYPES_COUNT - CTSNMT_START),

    firstPlayer_ (nullptr),
    secondPlayer_ (nullptr)
{
    incomingNetworkMessageProcessors_ [CTSNMT_ADD_ORDER - CTSNMT_START] = IncomingNetworkMessageProcessors::AddOrder;
    incomingNetworkMessageProcessors_ [CTSNMT_SPAWN_UNIT - CTSNMT_START] = IncomingNetworkMessageProcessors::SpawnUnit;

    SubscribeToEvent (Urho3D::E_CLIENTCONNECTED, URHO3D_HANDLER (ServerActivity, HandleClientConnected));
    SubscribeToEvent (Urho3D::E_CLIENTIDENTITY, URHO3D_HANDLER (ServerActivity, HandleClientIdentity));
    SubscribeToEvent (Urho3D::E_CLIENTDISCONNECTED, URHO3D_HANDLER (ServerActivity, HandleClientDisconnected));
    SubscribeToEvent (Urho3D::E_NETWORKMESSAGE, URHO3D_HANDLER (ServerActivity, HandleNetworkMessage));
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
    network->StartServer (SERVER_PORT);
}

void ServerActivity::Update (float timeStep)
{
    if (managersHub_ != nullptr)
    {
        managersHub_->HandleUpdate (timeStep);

        GameStatus newStatus = dynamic_cast <UnitsManager *> (managersHub_->GetManager (MI_UNITS_MANAGER))->CheckGameStatus ();
        if (newStatus != currentGameStatus_)
        {
            currentGameStatus_ = newStatus;
            ReportGameStatus (currentGameStatus_);
        }
    }

    // TODO: It's temporary. Autostarts game when 2 or more players are connected.
    else if (identifiedConnections_.Size () >= 2)
    {
        managersHub_ = new ManagersHub (scene_);
        firstPlayer_ = identifiedConnections_.Front ().first_;
        secondPlayer_ = identifiedConnections_.Back ().first_;

        unsigned int startCoins;
        LoadResources (startCoins);
        SetupPlayers (startCoins);
    }
}

void ServerActivity::Stop ()
{
    Urho3D::Network *network = context_->GetSubsystem <Urho3D::Network> ();
    network->StopServer ();
}

const Urho3D::String &ServerActivity::GetMapName () const
{
    return mapName_;
}

void ServerActivity::SetMapName (const Urho3D::String &mapName)
{
    mapName_ = mapName;
}

void ServerActivity::HandleClientConnected (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData)
{
    Urho3D::Connection *connection =
            dynamic_cast <Urho3D::Connection *> (eventData[Urho3D::ClientConnected::P_CONNECTION].GetPtr ());
    unidentifiedConnections_.Push (Urho3D::MakePair (connection, AUTO_DISCONNECT_TIME));
}

void ServerActivity::HandleClientIdentity (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData)
{
    Urho3D::Connection *connection =
            dynamic_cast <Urho3D::Connection *> (eventData[Urho3D::ClientConnected::P_CONNECTION].GetPtr ());
    Urho3D::String name = eventData [IdentityFields::NAME].GetString ();

    RemoveUnidentifiedConnection (connection);
    identifiedConnections_ [connection] = name;

    Urho3D::VectorBuffer data;
    data.WriteInt (currentGameStatus_);
    connection->SendMessage (STCNMT_GAME_STATUS, true, false, data);
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
    if (messageId < 0 || messageId >= CTSNMT_TYPES_COUNT)
    {
        throw UniversalException <ServerActivity> ("ServerActivity: received message with incorrect id " +
            Urho3D::String (messageId) + "!");
    }

    if (messageId >= CTSNMT_START)
    {
        incomingNetworkMessageProcessors_[messageId - CTSNMT_START] (managersHub_, identifiedConnections_,
                static_cast <Urho3D::Connection *> (eventData[Urho3D::NetworkMessage::P_CONNECTION].GetPtr ()));
    }
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
        if (iterator->first_ == connection)
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
        connectionData.first_->SendMessage (STCNMT_GAME_STATUS, true, false, data);
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
    SendUnitsTypesXMLToPlayers (unitsTypesXMLFile);
}

void ServerActivity::SendUnitsTypesXMLToPlayers (const Urho3D::XMLFile *unitsTypesXMLFile) const
{
    Urho3D::VectorBuffer messageData;
    messageData.WriteString (unitsTypesXMLFile->ToString ("    "));
    firstPlayer_->SendMessage (STCNMT_UNITS_TYPES_XML, true, false, messageData);
    secondPlayer_->SendMessage (STCNMT_UNITS_TYPES_XML, true, false, messageData);
}

void ServerActivity::SetupPlayers (unsigned int startCoins)
{
    PlayersManager *playersManager = dynamic_cast <PlayersManager *> (managersHub_->GetManager (MI_PLAYERS_MANAGER));
    Player firstPlayer (managersHub_);
    firstPlayer.SetBelongingMaterialIndex (0);
    firstPlayer.SetCoins (startCoins);
    playersManager->SetFirstPlayer (firstPlayer);

    Player secondPlayer (managersHub_);
    secondPlayer.SetBelongingMaterialIndex (1);
    secondPlayer.SetCoins (startCoins);
    playersManager->SetSecondPlayer (secondPlayer);
}
}

