#include "LauncherApplication.hpp"
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Network/Network.h>

#include <Urho3D/IO/Log.h>
#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Resource/ResourceCache.h>

#include <CastlesStrategy/Client/MainMenu/MainMenuActivity.hpp>
#include <CastlesStrategy/Client/Ingame/IngameActivity.hpp>
#include <CastlesStrategy/Server/Activity/ServerActivity.hpp>
#include <CastlesStrategy/Shared/ActivitiesControlEvents.hpp>
#include <CastlesStrategy/Shared/Village/Village.hpp>
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
    CastlesStrategy::Village::RegisterObject (context_);
    
    Urho3D::Script *script = new Urho3D::Script (context_);
    context_->RegisterSubsystem (script);

    Urho3D::Input *input = context_->GetSubsystem <Urho3D::Input> ();
    input->SetMouseVisible (true);
    input->SetMouseMode (Urho3D::MM_FREE);

    UIResizer::RegisterObject (context_);
    SubscribeToEvents ();
    SendEvent (CastlesStrategy::E_START_MAIN_MENU);

#ifndef NDEBUG
    Urho3D::ResourceCache *resourceCache = GetSubsystem <Urho3D::ResourceCache>();
    Urho3D::XMLFile *defaultUIStyle = resourceCache->GetResource <Urho3D::XMLFile> ("UI/DefaultStyle.xml");

    Urho3D::DebugHud *debugHud = new Urho3D::DebugHud (context_);
    context_->RegisterSubsystem (debugHud);

    debugHud->SetDefaultStyle (defaultUIStyle);
    debugHud->SetMode (Urho3D::DEBUGHUD_SHOW_ALL);
#endif
}

void LauncherApplication::Stop ()
{
    ActivitiesApplication::Stop ();
}

void LauncherApplication::SubscribeToEvents ()
{
    SubscribeToEvent (Urho3D::E_KEYUP, URHO3D_HANDLER (LauncherApplication, HandleKeyPress));
    SubscribeToEvent (CastlesStrategy::E_SHUTDOWN_ALL_ACTIVITIES, URHO3D_HANDLER (LauncherApplication, HandleShutdownAllActivities));
    SubscribeToEvent (CastlesStrategy::E_START_MAIN_MENU, URHO3D_HANDLER (LauncherApplication, HandleStartMainMenu));
    SubscribeToEvent (CastlesStrategy::E_START_CLIENT, URHO3D_HANDLER (LauncherApplication, HandleStartClient));
    SubscribeToEvent (CastlesStrategy::E_START_SERVER, URHO3D_HANDLER (LauncherApplication, HandleStartServer));
}

void LauncherApplication::HandleKeyPress (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
#ifndef NDEBUG
    if (eventData [Urho3D::KeyUp::P_KEY].GetInt () == Urho3D::KEY_F1)
    {
        context_->GetSubsystem <Urho3D::DebugHud> ()->ToggleAll ();
    }
#endif
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
            eventData [CastlesStrategy::StartClient::PORT].GetUInt (),
            eventData [CastlesStrategy::StartClient::IS_ADMIN].GetBool ()
    );
    SetupActivityNextFrame (client);
}

void LauncherApplication::HandleStartServer (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    CastlesStrategy::ServerActivity *server = new CastlesStrategy::ServerActivity (context_);
    // TODO: Select a map.
    server->SetMapName ("Default");
    SetupActivityNextFrame (server);
}
