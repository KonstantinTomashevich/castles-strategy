#pragma once
#include <ActivitiesApplication/Activity.hpp>
#include <Urho3D/Scene/Scene.h>

#include <CastlesStrategy/Shared/PlayerType.hpp>
#include <CastlesStrategy/Client/Ingame/IngameUIManager.hpp>
#include <CastlesStrategy/Client/Ingame/NetworkManager.hpp>
#include <CastlesStrategy/Client/Ingame/DataManager.hpp>
#include <CastlesStrategy/Client/Ingame/CameraManager.hpp>

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

    PlayerType GetPlayerType () const;
    void SetPlayerType (PlayerType playerType);

    const Urho3D::String &GetServerAddress () const;
    unsigned int GetPort () const;
    Urho3D::Scene *GetScene () const;

    IngameUIManager *GetIngameUIManager () const;
    NetworkManager *GetNetworkManager () const;
    DataManager *GetDataManager () const;
    CameraManager *GetCameraManager () const;

private:
    void InitScene () const;
    void SubscribeToEvents ();
    void ConnectToServer () const;

    void HandleConnectFailed (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleServerConnected (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleServerDisconnected (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

    PlayerType playerType_;
    Urho3D::String playerName_;
    Urho3D::String serverAddress_;
    unsigned int port_;
    Urho3D::Scene *scene_;

    IngameUIManager *ingameUIManager_;
    NetworkManager *networkManager_;
    DataManager *dataManager_;
    CameraManager *cameraManager_;
};
}

