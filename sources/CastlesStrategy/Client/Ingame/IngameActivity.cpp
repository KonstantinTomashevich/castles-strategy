#include "IngameActivity.hpp"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/IO/Log.h>

#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/StaticModel.h>

#include <Utils/UIResizer.hpp>
#include <CastlesStrategy/Shared/Network/ServerConstants.hpp>
#include <CastlesStrategy/Shared/ChangeActivityEvents.hpp>
#include <Urho3D/Graphics/StaticModel.h>

namespace CastlesStrategy
{
IngameActivity::IngameActivity (Urho3D::Context *context, const Urho3D::String &playerName, const Urho3D::String &serverAddress,
                                unsigned int port)
        : ActivitiesApplication::Activity (context),
          playerType_ (PT_OBSERVER),
          playerName_ (playerName),
          serverAddress_ (serverAddress),
          port_ (port),
          scene_ (new Urho3D::Scene (context)),

          ingameUIManager_ (nullptr),
          networkManager_ (nullptr),
          dataManager_ (nullptr),
          cameraManager_ (nullptr),
          fogOfWarManager_ (nullptr)
{

}

IngameActivity::~IngameActivity ()
{
    delete scene_;
    delete ingameUIManager_;
    delete networkManager_;
    delete dataManager_;
    delete cameraManager_;
    delete fogOfWarManager_;
}

void IngameActivity::Start ()
{
    ingameUIManager_ = new IngameUIManager (this);
    ingameUIManager_->LoadUI ();

    networkManager_ = new NetworkManager (this);
    dataManager_ = new DataManager (this);

    SubscribeToEvents ();
    ConnectToServer ();

    cameraManager_ = new CameraManager (this);
    fogOfWarManager_ = new FogOfWarManager (this);
    InitScene ();
}

void IngameActivity::Update (float timeStep)
{
    cameraManager_->Update (timeStep);
    dataManager_->Update (timeStep);
    fogOfWarManager_->Update (timeStep);
}

void IngameActivity::Stop ()
{
    ingameUIManager_->ClearUI ();
}

PlayerType IngameActivity::GetPlayerType () const
{
    return playerType_;
}

void IngameActivity::SetPlayerType (PlayerType playerType)
{
    playerType_ = playerType;
}

GameStatus IngameActivity::GetGameStatus () const
{
    return gameStatus_;
}

void IngameActivity::SetGameStatus (GameStatus gameStatus)
{
    gameStatus_ = gameStatus;
    if (gameStatus == GS_FIRST_WON || gameStatus == GS_SECOND_WON)
    {
        ingameUIManager_->InformGameEnded (gameStatus == GS_FIRST_WON);
        fogOfWarManager_->SetFogOfWarEnabled (false);
    }
}

const Urho3D::String &IngameActivity::GetServerAddress () const
{
    return serverAddress_;
}

unsigned int IngameActivity::GetPort () const
{
    return port_;
}

Urho3D::Scene *IngameActivity::GetScene () const
{
    return scene_;
}

IngameUIManager *IngameActivity::GetIngameUIManager () const
{
    return ingameUIManager_;
}

NetworkManager *IngameActivity::GetNetworkManager () const
{
    return networkManager_;
}

DataManager *IngameActivity::GetDataManager () const
{
    return dataManager_;
}

CameraManager *IngameActivity::GetCameraManager () const
{
    return cameraManager_;
}

FogOfWarManager *IngameActivity::GetFogOfWarManager () const
{
    return fogOfWarManager_;
}

void IngameActivity::InitScene () const
{
    scene_->CreateComponent <Urho3D::Octree> (Urho3D::LOCAL);
    scene_->CreateComponent <UIResizer> (Urho3D::LOCAL);
}

void IngameActivity::SubscribeToEvents ()
{
    SubscribeToEvent (Urho3D::E_CONNECTFAILED, URHO3D_HANDLER (IngameActivity, HandleConnectFailed));
    SubscribeToEvent (Urho3D::E_SERVERCONNECTED, URHO3D_HANDLER (IngameActivity, HandleServerConnected));
    SubscribeToEvent (Urho3D::E_SERVERDISCONNECTED, URHO3D_HANDLER (IngameActivity, HandleServerDisconnected));
}

void IngameActivity::ConnectToServer () const
{
    Urho3D::VariantMap identity;
    identity [IdentityFields::NAME] = playerName_;

    Urho3D::Network *network = context_->GetSubsystem <Urho3D::Network> ();
    network->Connect (serverAddress_, port_, scene_, identity);
}

void IngameActivity::HandleConnectFailed (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    ingameUIManager_->ShowMessage ("Connection failed!", "Couldn't connect to specified server!", "Go to main menu.",
                            [] (IngameActivity *activity) -> void
                            {
                                activity->SendEvent (SHUTDOWN_ALL_ACTIVITIES);
                                activity->SendEvent (START_MAIN_MENU);
                            });
}

void IngameActivity::HandleServerConnected (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    // TODO: Implement.
}

void IngameActivity::HandleServerDisconnected (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    if (gameStatus_ != GS_FIRST_WON && gameStatus_ != GS_SECOND_WON)
    {
        ingameUIManager_->ShowMessage ("Disconnected!", "Lost connection to server!", "Go to main menu.",
                [] (IngameActivity *activity) -> void
                {
                    activity->SendEvent (SHUTDOWN_ALL_ACTIVITIES);
                    activity->SendEvent (START_MAIN_MENU);
                });
    }
}
}
