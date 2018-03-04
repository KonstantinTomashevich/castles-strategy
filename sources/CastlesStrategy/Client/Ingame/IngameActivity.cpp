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
          playerName_ (playerName),
          serverAddress_ (serverAddress),
          port_ (port),
          scene_ (new Urho3D::Scene (context)),

          ingameUI_ (nullptr),
          networkMessagesProcessor_ (nullptr),
          dataProcessor_ (nullptr),
          cameraHandler_ (nullptr)
{

}

IngameActivity::~IngameActivity ()
{
    delete scene_;
    delete ingameUI_;
    delete networkMessagesProcessor_;
    delete dataProcessor_;
    delete cameraHandler_;
}

void IngameActivity::Start ()
{
    ingameUI_ = new IngameUI (this);
    ingameUI_->LoadUI ();

    networkMessagesProcessor_ = new NetworkMessagesProcessor (this);
    dataProcessor_ = new DataProcessor (this);

    SubscribeToEvents ();
    ConnectToServer ();

    cameraHandler_ = new CameraHandler (this);
    InitScene ();
}

void IngameActivity::Update (float timeStep)
{
    cameraHandler_->Update (timeStep);
}

void IngameActivity::Stop ()
{
    ingameUI_->ClearUI ();
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

IngameUI *IngameActivity::GetIngameUI () const
{
    return ingameUI_;
}

NetworkMessagesProcessor *IngameActivity::GetNetworkMessagesProcessor () const
{
    return networkMessagesProcessor_;
}

DataProcessor *IngameActivity::GetDataProcessor () const
{
    return dataProcessor_;
}

CameraHandler *IngameActivity::GetCameraHandler () const
{
    return cameraHandler_;
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
    ingameUI_->ShowMessage ("Connection failed!", "Couldn't connect to specified server!", "Go to main menu.",
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
    // TODO: Implement.
}
}
