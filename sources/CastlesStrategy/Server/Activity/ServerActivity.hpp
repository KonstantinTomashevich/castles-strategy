#pragma once
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Scene/Scene.h>

#include <CastlesStrategy/Server/Managers/ManagersHub.hpp>
#include <CastlesStrategy/Shared/Network/GameStatus.hpp>
#include <CastlesStrategy/Shared/PlayerType.hpp>
#include <ActivitiesApplication/Activity.hpp>

namespace CastlesStrategy
{
class ServerActivity;
typedef void (*ServerIncomingNetworkMessageProcessor) (ServerActivity *activity,
        Urho3D::VectorBuffer &messageData, Urho3D::Connection *sender);

class ServerActivity : public ActivitiesApplication::Activity
{
URHO3D_OBJECT (ServerActivity, Activity)
public:
    struct PlayerData
    {
        Urho3D::Connection *connection_;
        PlayerType playerType;
        bool readyForStart_;
    };

    typedef Urho3D::PODVector <Urho3D::Pair <Urho3D::Connection *, float> > UnidentifiedConnectionsVector;
    typedef Urho3D::HashMap <Urho3D::String, ServerActivity::PlayerData> IdentifiedConnectionsMap;

    ServerActivity (Urho3D::Context *context);
    virtual ~ServerActivity ();

    virtual void Start ();
    virtual void Update (float timeStep);
    virtual void Stop ();

    void ProcessRequestToChangeType (Urho3D::Connection *sender, PlayerType newType);
    void SetIsPlayerReady (Urho3D::Connection *sender, bool isReady);

    const Urho3D::String &GetMapName () const;
    void SetMapName (const Urho3D::String &mapName);

    const IdentifiedConnectionsMap &GetIdentifiedConnections () const;
    ManagersHub *GetManagersHub () const;

    Urho3D::Connection *GetFirstPlayer () const;
    Urho3D::Connection *GetSecondPlayer () const;

private:
    void HandleClientConnected (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);
    void HandleClientIdentity (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);
    void HandleClientDisconnected (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);
    void HandleNetworkMessage (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);

    void HandleComponentAdded (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);
    void HandlePlayerUnitsPullSync (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);
    void HandlePlayerCoinsSync (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);
    void HandleGameEnded (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);

    void HandleRequestGameStart (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);
    void HandleRequestKickPlayer (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);

    bool RemoveUnidentifiedConnection (Urho3D::Connection *connection);
    Urho3D::String RemoveIdentifiedConnection (Urho3D::Connection *connection);
    void ReportGameStatus () const;
    void ProcessUnidentifiedConnections (float timeStep);

    void LoadResources (unsigned int &startCoins);
    void LoadScene (const Urho3D::String &mapFolder);
    void LoadMap (const Urho3D::String &mapFolder, unsigned int &startCoins, bool &useDefaultUnitTypes);
    void LoadUnitsTypesAndSpawns (const Urho3D::String &mapFolder, bool useDefaultUnitTypes);
    void SetupPlayers (unsigned int startCoins);
    
    void SendPlayerTypeToAllPlayers (const Urho3D::String &playerName);
    void SendPlayerTypeToAllPlayers (IdentifiedConnectionsMap::KeyValue &playerInfo);
    void CollectMapData ();

    float autoDisconnectTime_;
    unsigned int serverPort_;

    GameStatus currentGameStatus_;
    UnidentifiedConnectionsVector unidentifiedConnections_;
    IdentifiedConnectionsMap identifiedConnections_;

    ManagersHub *managersHub_;
    Urho3D::Scene *scene_;
    Urho3D::String mapName_;
    Urho3D::VectorBuffer mapData_;

    Urho3D::PODVector <ServerIncomingNetworkMessageProcessor> incomingNetworkMessageProcessors_;
    unsigned int countOfPlayers_;
    Urho3D::Connection *firstPlayer_;
    Urho3D::Connection *secondPlayer_;
};
}
