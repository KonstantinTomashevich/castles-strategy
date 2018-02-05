#include "LauncherApplication.hpp"
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Input/Input.h>

#include <CastlesStrategy/Client/MainMenu/MainMenuActivity.hpp>
#include <CastlesStrategy/Client/Ingame/IngameActivity.hpp>
#include <CastlesStrategy/Server/Activity/ServerActivity.hpp>
#include <CastlesStrategy/Shared/ChangeActivityEvents.hpp>

URHO3D_DEFINE_APPLICATION_MAIN (LauncherApplication)
LauncherApplication::LauncherApplication (Urho3D::Context *context) : ActivitiesApplication::ActivitiesApplication (context)
{

}

LauncherApplication::~LauncherApplication ()
{

}

void LauncherApplication::Setup ()
{
    ActivitiesApplication::Setup ();
    engineParameters_ [Urho3D::EP_FULL_SCREEN] = false;
    engineParameters_ [Urho3D::EP_WINDOW_RESIZABLE] = true;
    engineParameters_ [Urho3D::EP_LOG_NAME] = "CastlesStrategy.log";
    engineParameters_ [Urho3D::EP_WINDOW_TITLE] = "Castles Strategy";
}

void LauncherApplication::Start ()
{
    ActivitiesApplication::Start ();
    Urho3D::Input *input = context_->GetSubsystem <Urho3D::Input> ();
    input->SetMouseVisible (true);
    input->SetMouseMode (Urho3D::MM_FREE);

    UIResizer::RegisterObject (context_);
    SubscribeToEvents ();
    SendEvent (CastlesStrategy::START_MAIN_MENU);
}

void LauncherApplication::Stop ()
{
    ActivitiesApplication::Stop ();
}

void LauncherApplication::SubscribeToEvents ()
{
    SubscribeToEvent (CastlesStrategy::SHUTDOWN_ALL_ACTIVITIES, URHO3D_HANDLER (LauncherApplication, HandleShutdownAllActivities));
    SubscribeToEvent (CastlesStrategy::START_MAIN_MENU, URHO3D_HANDLER (LauncherApplication, HandleStartMainMenu));
    SubscribeToEvent (CastlesStrategy::START_CLIENT, URHO3D_HANDLER (LauncherApplication, HandleStartClient));
    SubscribeToEvent (CastlesStrategy::START_SERVER, URHO3D_HANDLER (LauncherApplication, HandleStartServer));
}

void LauncherApplication::HandleShutdownAllActivities (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    StopAllActivitiesNextFrame ();
}

void LauncherApplication::HandleStartMainMenu (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    CastlesStrategy::MainMenuActivity *mainMenu = new CastlesStrategy::MainMenuActivity (context_);
    SetupActivityNextFrame (mainMenu);
}

void LauncherApplication::HandleStartClient (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    CastlesStrategy::IngameActivity *client = new CastlesStrategy::IngameActivity (context_,
        eventData [CastlesStrategy::StartClient::PLAYER_NAME].GetString (),
        eventData [CastlesStrategy::StartClient::ADDRESS].GetString (),
        eventData [CastlesStrategy::StartClient::PORT].GetUInt ());
    SetupActivityNextFrame (client);
}

void LauncherApplication::HandleStartServer (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    CastlesStrategy::ServerActivity *server = new CastlesStrategy::ServerActivity (context_);
    SetupActivityNextFrame (server);
}
