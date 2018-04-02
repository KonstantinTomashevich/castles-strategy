#include "IngameUIManager.hpp"
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Text.h>

#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/ScrollView.h>
#include <Urho3D/UI/ScrollBar.h>
#include <Urho3D/UI/LineEdit.h>

#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Resource/XMLElement.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Texture2D.h>

#include <CastlesStrategy/Client/Ingame/IngameActivity.hpp>
#include <CastlesStrategy/Shared/ActivitiesControlEvents.hpp>
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
    chatWindow_ (nullptr),
    connectedPlayersWindow_ (nullptr),
    requestedMessages_ ()
{

}

IngameUIManager::~IngameUIManager ()
{
    UnsubscribeFromAllEvents ();
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
    GetSubsystem <Urho3D::UI> ()->Clear ();

    topBar_ = nullptr;
    menu_ = nullptr;
    messageWindow_ = nullptr;
    chatWindow_ = nullptr;
    connectedPlayersWindow_ = nullptr;
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

    unsigned int orderedCount = dataManager->GetPredictedOrdedUnitsCount (unitType);
    countElement->SetText (Urho3D::String (predictedUnitsInPull) +
            (orderedCount != 0 ? "(+" + Urho3D::String (orderedCount) + ")" : ""));

    unitPullElement->GetChild ("SpawnButton", false)->SetVisible (dataManager->GetSelectedSpawnNode () != nullptr &&
            predictedUnitsInPull > 0);

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

void IngameUIManager::InformGameEnded (bool firstWon)
{
    menu_->SetVisible (true);
    dynamic_cast <Urho3D::Text *> (
            menu_->GetChild ("Menu", false))->SetText (Urho3D::String (firstWon ? "Blue" : "Red") + " won!");
    menu_->GetChild ("CloseMenuButton", false)->SetVisible (false);
}

void IngameUIManager::AddNewChatMessage (const Urho3D::String &message)
{
    Urho3D::ScrollView *messagesView = dynamic_cast <Urho3D::ScrollView *> (chatWindow_->GetChild ("MessagesView", false));
    Urho3D::Text *messageText = messagesView->GetContentElement ()->CreateChild <Urho3D::Text> ("Message");

    messageText->SetStyleAuto ();
    messageText->SetText (message);
    messageText->AddTag ("UIResizer");

    messageText->SetVar ("VWidth", 0.95f);
    messageText->SetVar ("VHeight", 0.1f);
    messageText->SetVar ("VX", 0.0f);
    messageText->SetVar ("VY", 0.0f);

    messageText->SetVar ("WDep", "PW");
    messageText->SetVar ("HDep", "SH");
    messageText->SetVar ("XDep", "SH");
    messageText->SetVar ("YDep", "SH");

    messageText->SetVar ("VTextSize", 0.015f);
    messageText->SetVar ("TSDep", "SH");
    messageText->SetWordwrap (true);

    SendEvent (EVENT_UI_RESIZER_RECALCULATE_UI_REQUEST);
    int contentHeight = messagesView->GetContentElement ()->GetHeight ();
    int scrollBarHeight = messagesView->GetVerticalScrollBar ()->GetHeight ();
    messagesView->SetViewPosition (0, scrollBarHeight > contentHeight ? 0 : contentHeight - scrollBarHeight);
}

void IngameUIManager::UpdatePlayersList ()
{
    Urho3D::ScrollView *playersView =
            dynamic_cast <Urho3D::ScrollView *> (connectedPlayersWindow_->GetChild ("PlayersView", false));
    Urho3D::PODVector <Urho3D::UIElement *> listElements;
    playersView->GetContentElement ()->GetChildren (listElements);

    Urho3D::ResourceCache *resourceCache = context_->GetSubsystem <Urho3D::ResourceCache> ();
    const Urho3D::HashMap <Urho3D::String, DataManager::PlayerData> &players = owner_->GetDataManager ()->GetPlayers ();

    while (listElements.Size () < players.Size ())
    {
        Urho3D::UIElement *listElement = playersView->GetContentElement ()->LoadChildXML (
                resourceCache->GetResource <Urho3D::XMLFile> ("UI/PlayersListElement.xml")->GetRoot (),
                playersView->GetDefaultStyle (true)
        );

        SubscribeToEvent (listElement->GetChild ("ToggleRoleButton", false), Urho3D::E_CLICKEND,
                URHO3D_HANDLER (IngameUIManager, HandleConnectedPlayersToggleRoleClicked));

        SubscribeToEvent (listElement->GetChild ("ToggleReadyButton", false), Urho3D::E_CLICKEND,
                URHO3D_HANDLER (IngameUIManager, HandleConnectedPlayersToggleReadyClicked));

        SubscribeToEvent (listElement->GetChild ("KickButton", false), Urho3D::E_CLICKEND,
                URHO3D_HANDLER (IngameUIManager, HandleConnectedPlayersKickClicked));

        listElements.Push (listElement);
    }

    while (listElements.Size () > players.Size ())
    {
        listElements.Back ()->Remove ();
        listElements.Pop ();
    }

    unsigned int index = 0;
    bool readyForStart = true;
    unsigned int playersCount = 0;

    for (const auto &player : players)
    {
        readyForStart = readyForStart && player.second_.readyForStart_;
        if (player.second_.playerType_ == PT_REQUESTED_TO_BE_PLAYER)
        {
            playersCount++;
        }

        Urho3D::UIElement *listElement = listElements [index];
        dynamic_cast <Urho3D::Text *> (listElement->GetChild ("NicknameText", false))->SetText (player.first_);

        listElement->GetChild ("ToggleRoleButton", false)->SetEnabled (player.first_ == owner_->GetPlayerName ());
        listElement->GetChild ("ToggleRoleButton", false)->SetVar (BUTTON_PLAYER_NAME_VAR, player.first_);

        dynamic_cast <Urho3D::Text *> (listElement->GetChild ("ToggleRoleButton", false)->GetChild ("Text", false))->
                SetText (Urho3D::String ((player.second_.playerType_ == PT_OBSERVER ? "Observer" : "Player")) +
                        (player.first_ == owner_->GetPlayerName () ? " (Toggle)" : "")
                );

        listElement->GetChild ("ToggleReadyButton", false)->SetEnabled (player.first_ == owner_->GetPlayerName ());
        listElement->GetChild ("ToggleReadyButton", false)->SetVar (BUTTON_PLAYER_NAME_VAR, player.first_);

        dynamic_cast <Urho3D::Text *> (listElement->GetChild ("ToggleReadyButton", false)->GetChild ("Text", false))->
                SetText (Urho3D::String ((player.second_.readyForStart_ ? "Ready" : "Not ready")) +
                        (player.first_ == owner_->GetPlayerName () ? " (Toggle)" : "")
                );

        listElement->GetChild ("KickButton", false)->SetVisible (owner_->IsAdmin ());
        listElement->GetChild ("KickButton", false)->SetVar (BUTTON_PLAYER_NAME_VAR, player.first_);
        index++;
    }

    connectedPlayersWindow_->GetChild ("ControlButtons", false)->GetChild ("StartGameButton", false)->
            SetVisible (owner_->IsAdmin () && readyForStart && playersCount == 2);
    SendEvent (EVENT_UI_RESIZER_RECALCULATE_UI_REQUEST);
}

void IngameUIManager::SwitchToPlayingState ()
{
    connectedPlayersWindow_->SetVisible (false);
    topBar_->SetVisible (true);
}

void IngameUIManager::LoadElements ()
{
    Urho3D::ResourceCache *resourceCache = context_->GetSubsystem <Urho3D::ResourceCache> ();
    Urho3D::UI *ui = context_->GetSubsystem <Urho3D::UI> ();

    ui->GetRoot ()->AddTag ("UIResizer");
    Urho3D::XMLFile *style = resourceCache->GetResource <Urho3D::XMLFile> ("UI/DefaultStyle.xml");

    topBar_ = dynamic_cast <Urho3D::Window *> (ui->GetRoot ()->LoadChildXML (
            resourceCache->GetResource <Urho3D::XMLFile> ("UI/TopBarWindow.xml")->GetRoot (), style));
    topBar_->SetVisible (false);

    menu_ = dynamic_cast <Urho3D::Window *> (ui->GetRoot ()->LoadChildXML (
            resourceCache->GetResource <Urho3D::XMLFile> ("UI/IngameMenuWindow.xml")->GetRoot (), style));
    menu_->SetVisible (false);

    messageWindow_ = dynamic_cast <Urho3D::Window *> (ui->GetRoot ()->LoadChildXML (
            resourceCache->GetResource <Urho3D::XMLFile> ("UI/MessageWindow.xml")->GetRoot (), style));
    messageWindow_->SetVisible (false);

    chatWindow_ = dynamic_cast <Urho3D::Window *> (ui->GetRoot ()->LoadChildXML (
            resourceCache->GetResource <Urho3D::XMLFile> ("UI/ChatWindow.xml")->GetRoot (), style));
    dynamic_cast <Urho3D::ScrollView *> (chatWindow_->GetChild ("MessagesView", false))->
            SetContentElement (chatWindow_->GetChild ("MessagesView", false)->GetChild ("MessagesContent", false));

    connectedPlayersWindow_ = dynamic_cast <Urho3D::Window *> (ui->GetRoot ()->LoadChildXML (
            resourceCache->GetResource <Urho3D::XMLFile> ("UI/ConnectedPlayersWindow.xml")->GetRoot (), style));
    dynamic_cast <Urho3D::ScrollView *> (connectedPlayersWindow_->GetChild ("PlayersView", false))->
            SetContentElement (connectedPlayersWindow_->GetChild ("PlayersView", false)->GetChild ("PlayersList", false));
    connectedPlayersWindow_->GetChild ("ControlButtons", false)->GetChild ("StartGameButton", false)->SetVisible (false);
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

    SubscribeToChatWindowEvents ();
    SubscribeToConnectedPlayersWindowEvents ();
    SubscribeToEvent (Urho3D::E_UIMOUSEDOUBLECLICK, URHO3D_HANDLER (IngameUIManager, HandleDoubleClickOnMap));
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

void IngameUIManager::SubscribeToChatWindowEvents ()
{
    Urho3D::Button *sendButton = dynamic_cast <Urho3D::Button *> (
            chatWindow_->GetChild ("MessageInput", false)->GetChild ("SendButton", false));
    SubscribeToEvent (sendButton, Urho3D::E_CLICKEND, URHO3D_HANDLER (IngameUIManager, HandleChatWindowSendClicked));
}

void IngameUIManager::SubscribeToConnectedPlayersWindowEvents ()
{
    Urho3D::Button *exitButton = dynamic_cast <Urho3D::Button *> (
            connectedPlayersWindow_->GetChild ("ControlButtons", false)->GetChild ("ExitButton", false));

    Urho3D::Button *startGameButton = dynamic_cast <Urho3D::Button *> (
            connectedPlayersWindow_->GetChild ("ControlButtons", false)->GetChild ("StartGameButton", false));

    SubscribeToEvent (exitButton, Urho3D::E_CLICKEND,
            URHO3D_HANDLER (IngameUIManager, HandleConnectedPlayersExitClicked));

    SubscribeToEvent (startGameButton, Urho3D::E_CLICKEND,
            URHO3D_HANDLER (IngameUIManager, HandleConnectedPlayersStartGameClicked));
}

void IngameUIManager::HandleTopBarMenuClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    menu_->SetVisible (true);
}

void IngameUIManager::HandleTopBarRecruitClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    Urho3D::UIElement *clicked = dynamic_cast <Urho3D::UIElement *> (eventData [Urho3D::ClickEnd::P_ELEMENT].GetPtr ());
    unsigned int unitType = clicked->GetVar (BUTTON_UNIT_TYPE_VAR).GetUInt ();
    owner_->GetDataManager ()->RecruitUnit (unitType);
}

void IngameUIManager::HandleTopBarSpawnClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    Urho3D::UIElement *clicked = dynamic_cast <Urho3D::UIElement *> (eventData [Urho3D::ClickEnd::P_ELEMENT].GetPtr ());
    unsigned int unitType = clicked->GetVar (BUTTON_UNIT_TYPE_VAR).GetUInt ();
    owner_->GetDataManager ()->SpawnUnit (unitType);
}

void IngameUIManager::HandleMenuCloseClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    menu_->SetVisible (false);
}

void IngameUIManager::HandleMenuExitToMainClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    SendEvent (E_SHUTDOWN_ALL_ACTIVITIES);
    SendEvent (E_START_MAIN_MENU);
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

void IngameUIManager::HandleChatWindowSendClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    Urho3D::LineEdit *messageEdit = dynamic_cast <Urho3D::LineEdit *> (
            chatWindow_->GetChild ("MessageInput", false)->GetChild ("Edit", false));

    owner_->GetNetworkManager ()->SendChatMessage (messageEdit->GetText ());
    messageEdit->SetText ("");
}

void IngameUIManager::HandleDoubleClickOnMap (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    if (eventData [Urho3D::UIMouseDoubleClick::P_ELEMENT].GetPtr () == nullptr)
    {
        owner_->GetDataManager ()->SetSelectedSpawnNode (owner_->GetCameraManager ()->RaycastNode (
                eventData [Urho3D::UIMouseDoubleClick::P_X].GetInt (),
                eventData [Urho3D::UIMouseDoubleClick::P_Y].GetInt (),
                true
        ));
    }
}

void IngameUIManager::HandleConnectedPlayersExitClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    SendEvent (E_SHUTDOWN_ALL_ACTIVITIES);
    SendEvent (E_START_MAIN_MENU);
}

void IngameUIManager::HandleConnectedPlayersStartGameClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    SendEvent (E_REQUEST_GAME_START);
}

void IngameUIManager::HandleConnectedPlayersToggleRoleClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    Urho3D::Button *pressedButton = dynamic_cast <Urho3D::Button *> (eventData [Urho3D::ClickEnd::P_ELEMENT].GetPtr ());
    if (pressedButton->GetVar (BUTTON_PLAYER_NAME_VAR).GetString () == owner_->GetPlayerName ())
    {
        owner_->GetNetworkManager ()->SendTogglePlayerTypeMessage ();
    }
}

void IngameUIManager::HandleConnectedPlayersToggleReadyClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    Urho3D::Button *pressedButton = dynamic_cast <Urho3D::Button *> (eventData [Urho3D::ClickEnd::P_ELEMENT].GetPtr ());
    if (pressedButton->GetVar (BUTTON_PLAYER_NAME_VAR).GetString () == owner_->GetPlayerName ())
    {
        owner_->GetNetworkManager ()->SendToggleReadyMessage ();
    }
}

void IngameUIManager::HandleConnectedPlayersKickClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    Urho3D::Button *pressedButton = dynamic_cast <Urho3D::Button *> (eventData [Urho3D::ClickEnd::P_ELEMENT].GetPtr ());
    if (owner_->IsAdmin ())
    {
        Urho3D::VariantMap kickEventData;
        kickEventData [RequestKickPlayer::PLAYER_NAME] = pressedButton->GetVar (BUTTON_PLAYER_NAME_VAR).GetString ();
        SendEvent (E_REQUEST_KICK_PLAYER, kickEventData);
    }
}
}
