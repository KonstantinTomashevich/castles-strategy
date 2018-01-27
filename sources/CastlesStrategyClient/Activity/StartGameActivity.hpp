#pragma once
#include <ActivitiesApplication/Activity.hpp>
#include <Utils/UIResizer.hpp>

namespace CastlesStrategy
{
class StartGameActivity : public ActivitiesApplication::Activity
{
public:
    StartGameActivity (Urho3D::Context *context);
    virtual ~StartGameActivity ();

    virtual void Start ();
    virtual void Update (float timeStep);
    virtual void Stop ();

private:
    void LoadUILayout ();
    void SubscribeToEvents ();

    void HandleConnectToServerClick (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleStartServerClick (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

    UIResizer *uiResizer_;
};

URHO3D_EVENT (CONNECT_TO_SERVER_REQUEST, ConnectToServerRequest)
{
    URHO3D_PARAM (PLAYER_NAME, PlayerName);
    URHO3D_PARAM (ADDRESS, Address);
    URHO3D_PARAM (PORT, Port);
}

URHO3D_EVENT (START_SERVER_REQUEST, StartServerRequest)
{
    URHO3D_PARAM (PLAYER_NAME, PlayerName);
}
}
