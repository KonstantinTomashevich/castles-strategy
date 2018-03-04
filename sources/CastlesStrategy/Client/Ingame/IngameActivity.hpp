#pragma once
#include <ActivitiesApplication/Activity.hpp>
#include <Urho3D/Scene/Scene.h>

#include <CastlesStrategy/Client/Ingame/IngameUI.hpp>
#include <CastlesStrategy/Client/Ingame/NetworkMessagesProcessor.hpp>
#include <CastlesStrategy/Client/Ingame/DataProcessor.hpp>
#include <CastlesStrategy/Client/Ingame/CameraHandler.hpp>

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
    Urho3D::Scene *GetScene () const;

    IngameUI *GetIngameUI () const;
    NetworkMessagesProcessor *GetNetworkMessagesProcessor () const;
    DataProcessor *GetDataProcessor () const;
    CameraHandler *GetCameraHandler () const;

private:
    void InitScene () const;
    void SubscribeToEvents ();
    void ConnectToServer () const;

    void HandleConnectFailed (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleServerConnected (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleServerDisconnected (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

    Urho3D::String playerName_;
    Urho3D::String serverAddress_;
    unsigned int port_;
    Urho3D::Scene *scene_;

    IngameUI *ingameUI_;
    NetworkMessagesProcessor *networkMessagesProcessor_;
    DataProcessor *dataProcessor_;
    CameraHandler *cameraHandler_;
};
}

