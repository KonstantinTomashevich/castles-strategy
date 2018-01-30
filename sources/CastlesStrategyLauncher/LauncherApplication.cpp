#include "LauncherApplication.hpp"
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Input/Input.h>
#include <CastlesStrategy/Client/MainMenu/MainMenuActivity.hpp>

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

    CastlesStrategy::MainMenuActivity *clientActivity = new CastlesStrategy::MainMenuActivity (context_);
    SetupActivityNextFrame (clientActivity);
}

void LauncherApplication::Stop ()
{
    ActivitiesApplication::Stop ();
}
