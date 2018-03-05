#include "IngameUI.hpp"
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UIEvents.h>

#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Resource/XMLElement.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Texture2D.h>

#include <CastlesStrategy/Client/Ingame/IngameActivity.hpp>
#include <CastlesStrategy/Shared/ChangeActivityEvents.hpp>
#include <Utils/UIResizer.hpp>

namespace CastlesStrategy
{
IngameUI::IngameUI (IngameActivity *owner) : Urho3D::Object (owner->GetContext ()),
    owner_ (owner),
    inputEnabled_ (true),

    topBar_ (nullptr),
    menu_ (nullptr),
    messageWindow_ (nullptr),
    requestedMessages_ ()
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
    DataProcessor *dataProcessor = owner_->GetDataProcessor ();
    for (unsigned int index = 0; index < dataProcessor->GetUnitsTypesCount (); index++)
    {
        if (index != dataProcessor->GetSpawnsUnitType ())
        {
            const UnitType &unitType = dataProcessor->GetUnitTypeByIndex (index);
            AddNewUnitTypeToTopBar (unitType);
        }
    }
    SendEvent (EVENT_UI_RESIZER_RECALCULATE_UI_REQUEST);
}

void IngameUI::ShowMessage (const Urho3D::String &title, const Urho3D::String &description, const Urho3D::String &okButtonText,
                          UICallback callback)
{
    requestedMessages_.Push ({title, description, okButtonText, callback});
    if (!messageWindow_->IsVisible ())
    {
        ShowNextMessage ();
    }
}

void IngameUI::ClearUI ()
{
    UnsubscribeFromAllEvents ();
    topBar_->Remove ();
    menu_->Remove ();
    messageWindow_->Remove ();

    topBar_ = nullptr;
    menu_ = nullptr;
    messageWindow_ = nullptr;
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

    messageWindow_ = dynamic_cast <Urho3D::Window *> (ui->GetRoot ()->LoadChildXML (
            resourceCache->GetResource <Urho3D::XMLFile> ("UI/MessageWindow.xml")->GetRoot (), style));
    messageWindow_->SetVisible (false);
}

void IngameUI::ShowNextMessage ()
{
    const MessageData &messageData = requestedMessages_.Front ();
    dynamic_cast <Urho3D::Text *> (messageWindow_->GetChild ("Title", false))->SetText (messageData.title);
    dynamic_cast <Urho3D::Text *> (messageWindow_->GetChild ("Description", false))->SetText (messageData.description);
    dynamic_cast <Urho3D::Text *> (messageWindow_->GetChild ("OkButton", false)->GetChild ("Text", false))->SetText (messageData.okButtonText);
    messageWindow_->SetVisible (true);
}

void IngameUI::AddNewUnitTypeToTopBar (const UnitType &unitType)
{
    Urho3D::ResourceCache *resourceCache = context_->GetSubsystem <Urho3D::ResourceCache> ();
    Urho3D::XMLFile *style = resourceCache->GetResource <Urho3D::XMLFile> ("UI/DefaultStyle.xml");

    Urho3D::UIElement *unitPullElement = topBar_->LoadChildXML (
            resourceCache->GetResource <Urho3D::XMLFile> ("UI/UnitPullElement.xml")->GetRoot (), style);

    Urho3D::BorderImage *iconElement = dynamic_cast <Urho3D::BorderImage *> (unitPullElement->GetChild ("Icon", false));
    iconElement->SetTexture (resourceCache->GetResource <Urho3D::Texture2D> (unitType.GetIconPath ()));
    iconElement->SetFullImageRect ();

    Urho3D::Button *recruitButton = dynamic_cast <Urho3D::Button *> (
            unitPullElement->GetChild ("RecruitButton", false));
    recruitButton->SetVar (BUTTON_UNIT_TYPE_VAR, unitType.GetId ());

    Urho3D::Button *spawnButton = dynamic_cast <Urho3D::Button *> (
            unitPullElement->GetChild ("SpawnButton", false));
    spawnButton->SetVar (BUTTON_UNIT_TYPE_VAR, unitType.GetId ());

    SubscribeToEvent (recruitButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (IngameUI, HandleTopBarRecruitClicked));
    SubscribeToEvent (spawnButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (IngameUI, HandleTopBarSpawnClicked));
}

void IngameUI::SubscribeToEvents ()
{
    SubscribeToTopBarEvents ();
    SubscribeToMenuEvents ();
    SubscribeToMessageWindowEvents ();
}

void IngameUI::SubscribeToTopBarEvents ()
{
    Urho3D::Button *menuButton = dynamic_cast <Urho3D::Button *> (topBar_->GetChild ("MenuButton", false));
    SubscribeToEvent (menuButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (IngameUI, HandleTopBarMenuClicked));
}

void IngameUI::SubscribeToMenuEvents ()
{
    Urho3D::Button *closeMenuButton = dynamic_cast <Urho3D::Button *> (menu_->GetChild ("CloseMenuButton", false));
    Urho3D::Button *exitToMainMenuButton = dynamic_cast <Urho3D::Button *> (menu_->GetChild ("ExitToMainMenuButton", false));
    Urho3D::Button *exitButton = dynamic_cast <Urho3D::Button *> (menu_->GetChild ("ExitButton", false));

    SubscribeToEvent (closeMenuButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (IngameUI, HandleMenuCloseClicked));
    SubscribeToEvent (exitToMainMenuButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (IngameUI, HandleMenuExitToMainClicked));
    SubscribeToEvent (exitButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (IngameUI, HandleMenuExitFromGameClicked));
}

void IngameUI::SubscribeToMessageWindowEvents ()
{
    Urho3D::Button *okButton = dynamic_cast <Urho3D::Button *> (messageWindow_->GetChild ("OkButton", false));
    SubscribeToEvent (okButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (IngameUI, HandleMessageWindowOkClicked));
}

void IngameUI::HandleTopBarMenuClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    menu_->SetVisible (true);
}

void IngameUI::HandleTopBarRecruitClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    // TODO: Implement.
}

void IngameUI::HandleTopBarSpawnClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    // TODO: Implement.
}

void IngameUI::HandleMenuCloseClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    menu_->SetVisible (false);
}

void IngameUI::HandleMenuExitToMainClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    SendEvent (SHUTDOWN_ALL_ACTIVITIES);
    SendEvent (START_MAIN_MENU);
}

void IngameUI::HandleMenuExitFromGameClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    context_->GetSubsystem <Urho3D::Engine> ()->Exit ();
}

void IngameUI::HandleMessageWindowOkClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    requestedMessages_.Front ().uiCallback (owner_);
    requestedMessages_.PopFront ();

    if (requestedMessages_.Empty ())
    {
        messageWindow_->SetVisible (false);
    }
    else
    {
        ShowNextMessage ();
    }
}
}
