#include "StartGameActivity.hpp"
#include <Urho3D/Core/Context.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>

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
    Urho3D::UI *ui = context_->GetSubsystem <Urho3D::UI> ();
    Urho3D::ResourceCache *resourceCache = context_->GetSubsystem <Urho3D::ResourceCache> ();

    ui->GetRoot ()->LoadChildXML (resourceCache->GetResource <Urho3D::XMLFile> ("UI/StartGameWindow.xml")->GetRoot (),
                                  resourceCache->GetResource <Urho3D::XMLFile> ("UI/DefaultStyle.xml"));
    uiResizer_->SetContinuousUpdate (false);
    uiResizer_->SetScanRootElement ("StartGameWindow");
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
}
