#include "LauncherApplication.hpp"
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/AngelScript/Script.h>

#include <CastlesStrategy/Client/MainMenu/MainMenuActivity.hpp>
#include <CastlesStrategy/Client/Ingame/IngameActivity.hpp>
#include <CastlesStrategy/Server/Activity/ServerActivity.hpp>
#include <CastlesStrategy/Shared/ChangeActivityEvents.hpp>
#include <Utils/UniversalException.hpp>

URHO3D_DEFINE_APPLICATION_MAIN (LauncherApplication)
void CustomTerminate ()
{
    try
    {
        std::rethrow_exception (std::current_exception ());
    }

    catch (AnyUniversalException &exception)
    {
        URHO3D_LOGERROR (exception.GetException ());
    }
    abort ();
}

LauncherApplication::LauncherApplication (Urho3D::Context *context) : ActivitiesApplication::ActivitiesApplication (context)
{

}

LauncherApplication::~LauncherApplication ()
{

}

void LauncherApplication::Setup ()
{
    Urho3D::String time = Urho3D::Time::GetTimeStamp ();
    time.Replace (':', ' ');
    std::set_terminate (CustomTerminate);
    ActivitiesApplication::Setup ();

    engineParameters_ [Urho3D::EP_FULL_SCREEN] = false;
    engineParameters_ [Urho3D::EP_WINDOW_RESIZABLE] = true;
    engineParameters_ [Urho3D::EP_LOG_NAME] = "CastlesStrategy " + time + ".log";
    engineParameters_ [Urho3D::EP_WINDOW_TITLE] = "Castles Strategy";
}

void LauncherApplication::Start ()
{
    ActivitiesApplication::Start ();
    UIResizer::RegisterObject (context_);
    CastlesStrategy::Unit::RegisterObject (context_);
    Urho3D::Script *script = new Urho3D::Script (context_);
    context_->RegisterSubsystem (script);

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
    // TODO: Select a map.
    server->SetMapName ("Default");
    SetupActivityNextFrame (server);
}
