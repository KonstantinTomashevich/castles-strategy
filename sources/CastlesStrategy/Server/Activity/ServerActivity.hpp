#pragma once
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Scene/Scene.h>

#include <ActivitiesApplication/Activity.hpp>
#include <CastlesStrategy/Shared/Network/GameStatus.hpp>
#include <CastlesStrategy/Server/Managers/ManagersHub.hpp>

namespace CastlesStrategy
{
typedef void (*ServerIncomingNetworkMessageProcessor) (ManagersHub *managersHub,
        const Urho3D::HashMap <Urho3D::Connection *, Urho3D::String> &identifiedConnections,
        Urho3D::Connection *sender);

class ServerActivity : public ActivitiesApplication::Activity
{
URHO3D_OBJECT (ServerActivity, Activity)
public:
    ServerActivity (Urho3D::Context *context);
    virtual ~ServerActivity ();

    virtual void Start ();
    virtual void Update (float timeStep);
    virtual void Stop ();

    const Urho3D::String &GetMapName () const;
    void SetMapName (const Urho3D::String &mapName);

private:
    void HandleClientConnected (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);
    void HandleClientIdentity (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);
    void HandleClientDisconnected (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);
    void HandleNetworkMessage (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);

    void HandleComponentAdded (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);
    void HandlePlayerUnitsPullSync (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);
    void HandlePlayerCoinsSync (Urho3D::StringHash eventHash, Urho3D::VariantMap &eventData);

    bool RemoveUnidentifiedConnection (Urho3D::Connection *connection);
    bool RemoveIdentifiedConnection (Urho3D::Connection *connection);
    void ReportGameStatus (GameStatus gameStatus) const;

    void LoadResources (unsigned int &startCoins);
    void LoadScene (const Urho3D::String &mapFolder);
    void LoadMap (const Urho3D::String &mapFolder, unsigned int &startCoins, bool &useDefaultUnitTypes);
    void SendMapPathToPlayers (const Urho3D::String &mapPath) const;
    void LoadUnitsTypesAndSpawns (const Urho3D::String &mapFolder, bool useDefaultUnitTypes);
    void SetupPlayers (unsigned int startCoins);

    GameStatus currentGameStatus_;
    Urho3D::PODVector <Urho3D::Pair <Urho3D::Connection *, float> > unidentifiedConnections_;
    Urho3D::HashMap <Urho3D::Connection *, Urho3D::String> identifiedConnections_;

    ManagersHub *managersHub_;
    Urho3D::Scene *scene_;
    Urho3D::String mapName_;
    Urho3D::PODVector <ServerIncomingNetworkMessageProcessor> incomingNetworkMessageProcessors_;

    Urho3D::Connection *firstPlayer_;
    Urho3D::Connection *secondPlayer_;
};
}
