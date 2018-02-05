#include "IngameUI.hpp"
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UIEvents.h>

#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Resource/XMLElement.h>
#include <Urho3D/Resource/ResourceCache.h>

#include <CastlesStrategy/Client/Ingame/IngameActivity.hpp>
#include <CastlesStrategy/Shared/ChangeActivityEvents.hpp>

namespace CastlesStrategy
{
IngameUI::IngameUI (IngameActivity *owner) : Urho3D::Object (owner->GetContext ()),
    owner_ (owner),
    inputEnabled_ (true),

    topBar_ (nullptr),
    menu_ (nullptr),
    errorWindow_ (nullptr),
    errorWindowOkCallback_ (nullptr)
{

}

IngameUI::~IngameUI ()
{

}

const IngameActivity *IngameUI::GetOwner () const
{
    return owner_;
}

bool IngameUI::IsInputEnabled () const
{
    return inputEnabled_;
}

void IngameUI::SetInputEnabled (bool inputEnabled)
{
    inputEnabled_ = inputEnabled;
}

void IngameUI::LoadUI ()
{
    LoadElements ();
    SubscribeToEvents ();
}

void IngameUI::SetupUnitsIcons ()
{
    // TODO: Implement.
}

void IngameUI::ShowError (const Urho3D::String &title, const Urho3D::String &description, UICallback callback)
{
    dynamic_cast <Urho3D::Text *> (errorWindow_->GetChild ("Title", false))->SetText (title);
    dynamic_cast <Urho3D::Text *> (errorWindow_->GetChild ("Description", false))->SetText (description);
    errorWindow_->SetVisible (true);
    errorWindowOkCallback_ = callback;
}

void IngameUI::ClearUI ()
{
    topBar_->Remove ();
    menu_->Remove ();
    errorWindow_->Remove ();

    topBar_ = nullptr;
    menu_ = nullptr;
    errorWindow_ = nullptr;
}

void IngameUI::LoadElements ()
{
    Urho3D::ResourceCache *resourceCache = context_->GetSubsystem <Urho3D::ResourceCache> ();
    Urho3D::UI *ui = context_->GetSubsystem <Urho3D::UI> ();

    ui->GetRoot ()->AddTag ("UIResizer");
    Urho3D::XMLFile *style = resourceCache->GetResource <Urho3D::XMLFile> ("UI/DefaultStyle.xml");

    topBar_ = dynamic_cast <Urho3D::Window *> (ui->GetRoot ()->LoadChildXML (
            resourceCache->GetResource <Urho3D::XMLFile> ("UI/TopBarWindow.xml")->GetRoot (), style));
    topBar_->SetVisible (true);

    menu_ = dynamic_cast <Urho3D::Window *> (ui->GetRoot ()->LoadChildXML (
            resourceCache->GetResource <Urho3D::XMLFile> ("UI/IngameMenuWindow.xml")->GetRoot (), style));
    menu_->SetVisible (false);

    errorWindow_ = dynamic_cast <Urho3D::Window *> (ui->GetRoot ()->LoadChildXML (
            resourceCache->GetResource <Urho3D::XMLFile> ("UI/ErrorWindow.xml")->GetRoot (), style));
    errorWindow_->SetVisible (false);
}

void IngameUI::SubscribeToEvents ()
{
    // TODO: TopBar events.
    // TODO: Menu events.

    SubscribeToErrorWindowEvents ();
}

void IngameUI::SubscribeToErrorWindowEvents ()
{
    Urho3D::UI *ui = context_->GetSubsystem <Urho3D::UI> ();
    Urho3D::Button *okButton = dynamic_cast <Urho3D::Button *> (errorWindow_->GetChild ("OkButton", false));
    SubscribeToEvent (okButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (IngameUI, HandleErrorWindowOkClicked));
}

void IngameUI::HandleErrorWindowOkClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    errorWindow_->SetVisible (false);
    errorWindowOkCallback_ (owner_);
}
}
