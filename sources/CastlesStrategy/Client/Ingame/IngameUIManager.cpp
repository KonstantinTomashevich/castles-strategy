#include "IngameUIManager.hpp"
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
#include <Utils/UniversalException.hpp>

namespace CastlesStrategy
{
IngameUIManager::IngameUIManager (IngameActivity *owner) : Urho3D::Object (owner->GetContext ()),
    owner_ (owner),
    inputEnabled_ (true),

    topBar_ (nullptr),
    menu_ (nullptr),
    messageWindow_ (nullptr),
    requestedMessages_ ()
{

}

IngameUIManager::~IngameUIManager ()
{

}

const IngameActivity *IngameUIManager::GetOwner () const
{
    return owner_;
}

bool IngameUIManager::IsInputEnabled () const
{
    return inputEnabled_;
}

void IngameUIManager::SetInputEnabled (bool inputEnabled)
{
    inputEnabled_ = inputEnabled;
}

void IngameUIManager::LoadUI ()
{
    LoadElements ();
    SubscribeToEvents ();
}

void IngameUIManager::SetupUnitsIcons ()
{
    DataManager *dataProcessor = owner_->GetDataManager ();
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

void IngameUIManager::ShowMessage (const Urho3D::String &title, const Urho3D::String &description, const Urho3D::String &okButtonText,
                          UICallback callback)
{
    requestedMessages_.Push ({title, description, okButtonText, callback});
    if (!messageWindow_->IsVisible ())
    {
        ShowNextMessage ();
    }
}

void IngameUIManager::ClearUI ()
{
    UnsubscribeFromAllEvents ();
    topBar_->Remove ();
    menu_->Remove ();
    messageWindow_->Remove ();

    topBar_ = nullptr;
    menu_ = nullptr;
    messageWindow_ = nullptr;
}

void IngameUIManager::CheckUIForUnitsType (unsigned int unitType)
{
    Urho3D::UIElement *unitPullElement = topBar_->GetChild (("UnitsType" + Urho3D::String (unitType)));
    if (unitPullElement == nullptr)
    {
        throw UniversalException <IngameUIManager> ("IngameUIManager: requested to update units pull ui of type " +
                Urho3D::String (unitType) + " but ui element for this type is not exists!");
    }

    DataManager *dataManager = owner_->GetDataManager ();
    unsigned int predictedUnitsInPull = dataManager->GetPredictedUnitsInPull (unitType);
    Urho3D::Text *countElement = dynamic_cast <Urho3D::Text *> (unitPullElement->
            GetChild ("IconAndCountElement", false)->GetChild ("Count", false));

    countElement->SetText (Urho3D::String (predictedUnitsInPull));
    unitPullElement->GetChild ("SpawnButton", false)->SetVisible (predictedUnitsInPull > 0);
    unitPullElement->GetChild ("RecruitButton", false)->SetVisible (
            dataManager->GetUnitTypeByIndex (unitType).GetRecruitmentCost () <= dataManager->GetPredictedCoins ());
}

void IngameUIManager::UpdateCoins (unsigned int coins)
{
    DataManager *dataManager = owner_->GetDataManager ();
    dynamic_cast <Urho3D::Text *> (
            topBar_->GetChild ("CoinsElement", false)->GetChild ("Text", false))->SetText (Urho3D::String (coins));

    for (unsigned int index = 0; index < dataManager->GetUnitsTypesCount (); index++)
    {
        if (index != dataManager->GetSpawnsUnitType ())
        {
            CheckUIForUnitsType (index);
        }
    }
}

void IngameUIManager::LoadElements ()
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

void IngameUIManager::ShowNextMessage ()
{
    const MessageData &messageData = requestedMessages_.Front ();
    dynamic_cast <Urho3D::Text *> (messageWindow_->GetChild ("Title", false))->SetText (messageData.title);
    dynamic_cast <Urho3D::Text *> (messageWindow_->GetChild ("Description", false))->SetText (messageData.description);
    dynamic_cast <Urho3D::Text *> (messageWindow_->GetChild ("OkButton", false)->GetChild ("Text", false))->
            SetText (messageData.okButtonText);
    messageWindow_->SetVisible (true);
}

void IngameUIManager::AddNewUnitTypeToTopBar (const UnitType &unitType)
{
    Urho3D::ResourceCache *resourceCache = context_->GetSubsystem <Urho3D::ResourceCache> ();
    Urho3D::XMLFile *style = resourceCache->GetResource <Urho3D::XMLFile> ("UI/DefaultStyle.xml");

    Urho3D::UIElement *unitPullElement = topBar_->LoadChildXML (
            resourceCache->GetResource <Urho3D::XMLFile> ("UI/UnitPullElement.xml")->GetRoot (), style);
    unitPullElement->SetName ("UnitsType" + Urho3D::String (unitType.GetId ()));

    Urho3D::BorderImage *iconElement = dynamic_cast <Urho3D::BorderImage *> (unitPullElement->
            GetChild ("IconAndCountElement", false)->GetChild ("Icon", false));
    iconElement->SetTexture (resourceCache->GetResource <Urho3D::Texture2D> (unitType.GetIconPath ()));
    iconElement->SetFullImageRect ();

    Urho3D::Button *recruitButton = dynamic_cast <Urho3D::Button *> (
            unitPullElement->GetChild ("RecruitButton", false));
    recruitButton->SetVar (BUTTON_UNIT_TYPE_VAR, unitType.GetId ());

    Urho3D::Button *spawnButton = dynamic_cast <Urho3D::Button *> (
            unitPullElement->GetChild ("SpawnButton", false));
    spawnButton->SetVar (BUTTON_UNIT_TYPE_VAR, unitType.GetId ());

    CheckUIForUnitsType (unitType.GetId ());
    SubscribeToEvent (recruitButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (IngameUIManager, HandleTopBarRecruitClicked));
    SubscribeToEvent (spawnButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (IngameUIManager, HandleTopBarSpawnClicked));
}

void IngameUIManager::SubscribeToEvents ()
{
    SubscribeToTopBarEvents ();
    SubscribeToMenuEvents ();
    SubscribeToMessageWindowEvents ();
}

void IngameUIManager::SubscribeToTopBarEvents ()
{
    Urho3D::Button *menuButton = dynamic_cast <Urho3D::Button *> (topBar_->GetChild ("MenuButton", false));
    SubscribeToEvent (menuButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (IngameUIManager, HandleTopBarMenuClicked));
}

void IngameUIManager::SubscribeToMenuEvents ()
{
    Urho3D::Button *closeMenuButton = dynamic_cast <Urho3D::Button *> (menu_->GetChild ("CloseMenuButton", false));
    Urho3D::Button *exitToMainMenuButton = dynamic_cast <Urho3D::Button *> (menu_->GetChild ("ExitToMainMenuButton", false));
    Urho3D::Button *exitButton = dynamic_cast <Urho3D::Button *> (menu_->GetChild ("ExitButton", false));

    SubscribeToEvent (closeMenuButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (IngameUIManager, HandleMenuCloseClicked));
    SubscribeToEvent (exitToMainMenuButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (IngameUIManager, HandleMenuExitToMainClicked));
    SubscribeToEvent (exitButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (IngameUIManager, HandleMenuExitFromGameClicked));
}

void IngameUIManager::SubscribeToMessageWindowEvents ()
{
    Urho3D::Button *okButton = dynamic_cast <Urho3D::Button *> (messageWindow_->GetChild ("OkButton", false));
    SubscribeToEvent (okButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (IngameUIManager, HandleMessageWindowOkClicked));
}

void IngameUIManager::HandleTopBarMenuClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    menu_->SetVisible (true);
}

void IngameUIManager::HandleTopBarRecruitClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    // TODO: Implement.
}

void IngameUIManager::HandleTopBarSpawnClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    // TODO: Implement.
}

void IngameUIManager::HandleMenuCloseClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    menu_->SetVisible (false);
}

void IngameUIManager::HandleMenuExitToMainClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    SendEvent (SHUTDOWN_ALL_ACTIVITIES);
    SendEvent (START_MAIN_MENU);
}

void IngameUIManager::HandleMenuExitFromGameClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    context_->GetSubsystem <Urho3D::Engine> ()->Exit ();
}

void IngameUIManager::HandleMessageWindowOkClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
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
