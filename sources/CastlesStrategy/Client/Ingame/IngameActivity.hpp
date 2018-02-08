#pragma once
#include <ActivitiesApplication/Activity.hpp>
#include <Urho3D/Scene/Scene.h>
#include <CastlesStrategy/Client/Ingame/IngameUI.hpp>

namespace CastlesStrategy
{
class IngameActivity : public ActivitiesApplication::Activity
{
URHO3D_OBJECT (IngameActivity, Activity)
public:
    IngameActivity (Urho3D::Context *context, const Urho3D::String &playerName, const Urho3D::String &serverAddress,
                        unsigned int port);
    virtual ~IngameActivity ();

    virtual void Start ();
    virtual void Update (float timeStep);
    virtual void Stop ();

    const Urho3D::String &GetServerAddress () const;
    unsigned int GetPort () const;
    const Urho3D::Scene *GetScene () const;

private:
    void InitScene () const;
    void SubscribeToEvents ();
    void ConnectToServer () const;
    void SetupViewport () const;

    void HandleConnectFailed (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleServerConnected (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleServerDisconnected (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleNetworkMessage (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

    Urho3D::String playerName_;
    Urho3D::String serverAddress_;
    unsigned int port_;
    Urho3D::Scene *scene_;

    IngameUI *ingameUI_;
};
}
