#pragma once
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Network/Connection.h>
#include <Urho3D/Scene/Scene.h>

#include <ActivitiesApplication/Activity.hpp>
#include <CastlesStrategy/Server/Managers/GameStatus.hpp>
#include <CastlesStrategy/Server/Managers/ManagersHub.hpp>

namespace CastlesStrategy
{
// TODO: Maybe allow to specify this vars in server activity (except P_IDENTITY_NAME of course).
const float AUTO_DISCONNECT_TIME = 1.0f;
const unsigned int SERVER_PORT = 10001;
const Urho3D::StringHash P_IDENTITY_NAME ("Name");
const char *DEFAULT_MAPS_FOLDER = "Maps";
const char *DEFAULT_UNITS_TYPES_PATH = "DefaultUnitsTypes.xml";

typedef void (*IncomingNetworkMessageProcessor) (ManagersHub *managersHub,
        const Urho3D::Vector <Urho3D::Pair <Urho3D::Connection *, Urho3D::String> > &identifiedConnections);

class ServerActivity : public ActivitiesApplication::Activity
{
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

    bool RemoveUnidentifiedConnection (Urho3D::Connection *connection);
    bool RemoveIdentifiedConnection (Urho3D::Connection *connection);
    void ReportGameStatus (GameStatus gameStatus) const;

    void LoadResources (unsigned int &startCoins);
    void SetupPlayers (unsigned int startCoins);

    GameStatus currentGameStatus_;
    Urho3D::PODVector <Urho3D::Pair <Urho3D::Connection *, float> > unidentifiedConnections_;
    Urho3D::Vector <Urho3D::Pair <Urho3D::Connection *, Urho3D::String> > identifiedConnections_;

    ManagersHub *managersHub_;
    Urho3D::Scene *scene_;
    Urho3D::String mapName_;
    Urho3D::PODVector <IncomingNetworkMessageProcessor> incomingNetworkMessageProcessors_;

    Urho3D::Connection *firstPlayer_;
    Urho3D::Connection *secondPlayer_;
};
}
