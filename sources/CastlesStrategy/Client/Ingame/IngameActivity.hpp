#pragma once
#include <ActivitiesApplication/Activity.hpp>
#include <Urho3D/Scene/Scene.h>

#include <CastlesStrategy/Shared/PlayerType.hpp>
#include <CastlesStrategy/Shared/Network/GameStatus.hpp>

#include <CastlesStrategy/Client/Ingame/IngameUIManager.hpp>
#include <CastlesStrategy/Client/Ingame/NetworkManager.hpp>
#include <CastlesStrategy/Client/Ingame/DataManager.hpp>
#include <CastlesStrategy/Client/Ingame/CameraManager.hpp>
#include <CastlesStrategy/Client/Ingame/FogOfWarManager.hpp>

namespace CastlesStrategy
{
class IngameActivity : public ActivitiesApplication::Activity
{
URHO3D_OBJECT (IngameActivity, Activity)
public:
    IngameActivity (Urho3D::Context *context, const Urho3D::String &playerName, const Urho3D::String &serverAddress,
                unsigned int port, bool isAdmin);
    virtual ~IngameActivity ();

    virtual void Start ();
    virtual void Update (float timeStep);
    virtual void Stop ();

    PlayerType GetPlayerType () const;
    void SetPlayerType (PlayerType playerType);

    GameStatus GetGameStatus () const;
    void SetGameStatus (GameStatus gameStatus);

    bool IsAdmin () const;
    const Urho3D::String &GetPlayerName () const;

    const Urho3D::String &GetServerAddress () const;
    unsigned int GetPort () const;
    Urho3D::Scene *GetScene () const;

    IngameUIManager *GetIngameUIManager () const;
    NetworkManager *GetNetworkManager () const;
    DataManager *GetDataManager () const;
    CameraManager *GetCameraManager () const;
    FogOfWarManager *GetFogOfWarManager () const;

private:
    void InitScene () const;
    void SubscribeToEvents ();
    void ConnectToServer () const;

    void HandleConnectFailed (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleServerConnected (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleServerDisconnected (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

    PlayerType playerType_;
    GameStatus gameStatus_;
    bool isAdmin_;

    Urho3D::String playerName_;
    Urho3D::String serverAddress_;
    unsigned int port_;
    Urho3D::Scene *scene_;

    IngameUIManager *ingameUIManager_;
    NetworkManager *networkManager_;
    DataManager *dataManager_;
    CameraManager *cameraManager_;
    FogOfWarManager *fogOfWarManager_;
};
}

