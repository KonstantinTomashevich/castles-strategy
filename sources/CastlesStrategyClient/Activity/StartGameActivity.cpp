#include "StartGameActivity.hpp"
#include <Urho3D/Core/Context.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>

#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/UIEvents.h>

namespace CastlesStrategy
{
StartGameActivity::StartGameActivity (Urho3D::Context *context) : ActivitiesApplication::Activity (context),
    uiResizer_ (new UIResizer (context))
{

}

StartGameActivity::~StartGameActivity ()
{
    delete uiResizer_;
}

void StartGameActivity::Start ()
{
    LoadUILayout ();
    SubscribeToEvents ();
}

void StartGameActivity::Update (float timeStep)
{
    uiResizer_->Update (timeStep);
}

void StartGameActivity::Stop ()
{
    Urho3D::UI *ui = context_->GetSubsystem <Urho3D::UI> ();
    ui->GetRoot ()->RemoveAllChildren ();
}

void StartGameActivity::LoadUILayout ()
{
    Urho3D::UI *ui = context_->GetSubsystem <Urho3D::UI> ();
    Urho3D::ResourceCache *resourceCache = context_->GetSubsystem <Urho3D::ResourceCache> ();

    ui->GetRoot ()->LoadChildXML (resourceCache->GetResource <Urho3D::XMLFile> ("UI/StartGameWindow.xml")->GetRoot (),
                                  resourceCache->GetResource <Urho3D::XMLFile> ("UI/DefaultStyle.xml"));
    uiResizer_->SetContinuousUpdate (false);
    uiResizer_->SetScanRootElement ("StartGameWindow");
}

void StartGameActivity::SubscribeToEvents ()
{
    Urho3D::UI *ui = context_->GetSubsystem <Urho3D::UI> ();
    Urho3D::Button *connectToServerButton = dynamic_cast <Urho3D::Button *> (
            ui->GetRoot ()->GetChild ("StartGameWindow", false)->GetChild ("SplitView", false)->
                    GetChild ("ConnectWindow", false)->GetChild ("ConnectButton", false));

    Urho3D::Button *startServerButton = dynamic_cast <Urho3D::Button *> (
            ui->GetRoot ()->GetChild ("StartGameWindow", false)->GetChild ("SplitView", false)->
                    GetChild ("StartServerWindow", false)->GetChild ("StartServerButton", false));

    SubscribeToEvent (connectToServerButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (StartGameActivity, HandleConnectToServerClick));
    SubscribeToEvent (startServerButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (StartGameActivity, HandleStartServerClick));
}

void StartGameActivity::HandleConnectToServerClick (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    Urho3D::UI *ui = context_->GetSubsystem <Urho3D::UI> ();
    Urho3D::String playerName = dynamic_cast <Urho3D::LineEdit *> (
            ui->GetRoot ()->GetChild ("StartGameWindow", false)->GetChild ("NameInput", false)->
                GetChild ("Edit", false))->GetText ();

    Urho3D::UIElement *connectWindow = ui->GetRoot ()->GetChild ("StartGameWindow", false)->
            GetChild ("SplitView", false)->GetChild ("ConnectWindow", false);

    Urho3D::String address = dynamic_cast <Urho3D::LineEdit *> (
            connectWindow->GetChild ("AddressInput", false)->GetChild ("Edit", false))->GetText ();

    unsigned int port = Urho3D::ToUInt (dynamic_cast <Urho3D::LineEdit *> (
            connectWindow->GetChild ("PortInput", false)->GetChild ("Edit", false))->GetText ());

    Urho3D::VariantMap newEventData;
    newEventData [ConnectToServerRequest::PLAYER_NAME] = playerName;
    newEventData [ConnectToServerRequest::ADDRESS] = address;
    newEventData [ConnectToServerRequest::PORT] = port;
    SendEvent (CONNECT_TO_SERVER_REQUEST, newEventData);
}

void StartGameActivity::HandleStartServerClick (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    Urho3D::UI *ui = context_->GetSubsystem <Urho3D::UI> ();
    Urho3D::String playerName = dynamic_cast <Urho3D::LineEdit *> (
            ui->GetRoot ()->GetChild ("StartGameWindow", false)->GetChild ("NameInput", false)->
                    GetChild ("Edit", false))->GetText ();

    Urho3D::VariantMap newEventData;
    newEventData [StartServerRequest::PLAYER_NAME] = playerName;
    SendEvent (START_SERVER_REQUEST, newEventData);
}
}
