#include "MainMenuActivity.hpp"
#include <Urho3D/Core/Context.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>

#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/UIEvents.h>

#include <CastlesStrategy/Shared/Network/ServerConstants.hpp>
#include <CastlesStrategy/Shared/ChangeActivityEvents.hpp>

namespace CastlesStrategy
{
MainMenuActivity::MainMenuActivity (Urho3D::Context *context) : ActivitiesApplication::Activity (context),
    uiResizer_ (new UIResizer (context))
{

}

MainMenuActivity::~MainMenuActivity ()
{
    delete uiResizer_;
}

void MainMenuActivity::Start ()
{
    LoadUILayout ();
    SubscribeToEvents ();
}

void MainMenuActivity::Update (float timeStep)
{
    uiResizer_->Update (timeStep);
}

void MainMenuActivity::Stop ()
{
    Urho3D::UI *ui = context_->GetSubsystem <Urho3D::UI> ();
    ui->GetRoot ()->RemoveAllChildren ();
}

void MainMenuActivity::LoadUILayout ()
{
    Urho3D::UI *ui = context_->GetSubsystem <Urho3D::UI> ();
    Urho3D::ResourceCache *resourceCache = context_->GetSubsystem <Urho3D::ResourceCache> ();

    ui->GetRoot ()->LoadChildXML (resourceCache->GetResource <Urho3D::XMLFile> ("UI/StartGameWindow.xml")->GetRoot (),
                                  resourceCache->GetResource <Urho3D::XMLFile> ("UI/DefaultStyle.xml"));
    uiResizer_->SetContinuousUpdate (false);
    uiResizer_->SetScanRootElement ("StartGameWindow");
}

void MainMenuActivity::SubscribeToEvents ()
{
    Urho3D::UI *ui = context_->GetSubsystem <Urho3D::UI> ();
    Urho3D::Button *connectToServerButton = dynamic_cast <Urho3D::Button *> (
            ui->GetRoot ()->GetChild ("StartGameWindow", false)->GetChild ("SplitView", false)->
                    GetChild ("ConnectWindow", false)->GetChild ("ConnectButton", false));

    Urho3D::Button *startServerButton = dynamic_cast <Urho3D::Button *> (
            ui->GetRoot ()->GetChild ("StartGameWindow", false)->GetChild ("SplitView", false)->
                    GetChild ("StartServerWindow", false)->GetChild ("StartServerButton", false));

    SubscribeToEvent (connectToServerButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (MainMenuActivity, HandleConnectToServerClick));
    SubscribeToEvent (startServerButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (MainMenuActivity, HandleStartServerClick));
}

void MainMenuActivity::HandleConnectToServerClick (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    SendEvent (SHUTDOWN_ALL_ACTIVITIES);
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

    Urho3D::VariantMap startClientData;
    startClientData [StartClient::PLAYER_NAME] = playerName;
    startClientData [StartClient::ADDRESS] = address;
    startClientData [StartClient::PORT] = port;
    SendEvent (START_CLIENT, startClientData);
}

void MainMenuActivity::HandleStartServerClick (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    SendEvent (SHUTDOWN_ALL_ACTIVITIES);
    Urho3D::UI *ui = context_->GetSubsystem <Urho3D::UI> ();
    Urho3D::String playerName = dynamic_cast <Urho3D::LineEdit *> (
            ui->GetRoot ()->GetChild ("StartGameWindow", false)->GetChild ("NameInput", false)->
                    GetChild ("Edit", false))->GetText ();

    Urho3D::VariantMap startServerEventData;
    SendEvent (START_SERVER);

    Urho3D::VariantMap startClientData;
    startClientData [StartClient::PLAYER_NAME] = playerName;
    startClientData [StartClient::ADDRESS] = "localhost";
    startClientData [StartClient::PORT] = DEFAULT_SERVER_PORT;
    SendEvent (START_CLIENT, startClientData);
}
}
