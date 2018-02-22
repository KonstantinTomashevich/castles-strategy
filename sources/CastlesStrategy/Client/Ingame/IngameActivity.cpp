#include "IngameActivity.hpp"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/NetworkEvents.h>

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/Application.h>

#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Camera.h>

#include <Utils/UIResizer.hpp>
#include <CastlesStrategy/Shared/Network/ServerConstants.hpp>
#include <CastlesStrategy/Shared/ChangeActivityEvents.hpp>

namespace CastlesStrategy
{
IngameActivity::IngameActivity (Urho3D::Context *context, const Urho3D::String &playerName, const Urho3D::String &serverAddress,
                                unsigned int port)
        : ActivitiesApplication::Activity (context),
          playerName_ (playerName),
          serverAddress_ (serverAddress),
          port_ (port),
          scene_ (new Urho3D::Scene (context)),

          ingameUI_ (nullptr)
{
    InitScene ();
    ingameUI_ = new IngameUI (this);
    networkMessagesProcessor_ = new NetworkMessagesProcessor (this);
    dataProcessor_ = new DataProcessor (this);
}

IngameActivity::~IngameActivity ()
{
    delete scene_;
    delete ingameUI_;
    delete networkMessagesProcessor_;
    delete dataProcessor_;
}

void IngameActivity::Start ()
{
    ingameUI_->LoadUI ();
    SubscribeToEvents ();
    ConnectToServer ();
    SetupViewport ();
}

void IngameActivity::Update (float timeStep)
{

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

const Urho3D::Scene *IngameActivity::GetScene () const
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

void IngameActivity::InitScene () const
{
    UIResizer *uiResizer = scene_->CreateComponent <UIResizer> (Urho3D::LOCAL);
    scene_->CreateComponent <Urho3D::Octree> (Urho3D::LOCAL);
    Urho3D::Camera *camera = scene_->CreateChild ("Camera", Urho3D::REPLICATED)->
            CreateComponent <Urho3D::Camera> (Urho3D::LOCAL);
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

void IngameActivity::SetupViewport () const
{
    Urho3D::Renderer *renderer = context_->GetSubsystem <Urho3D::Renderer> ();
    renderer->SetNumViewports (1);
    renderer->SetViewport (0, new Urho3D::Viewport (
            context_, scene_, scene_->GetChild ("Camera")->GetComponent <Urho3D::Camera> ()));
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
