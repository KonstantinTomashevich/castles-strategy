#pragma once
#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/XMLElement.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/Container/List.h>

namespace CastlesStrategy
{
class IngameActivity;
typedef void (*UICallback) (IngameActivity *activity);

class IngameUI : public Urho3D::Object
{
URHO3D_OBJECT (IngameUI, Object)
public:
    IngameUI (IngameActivity *owner);
    virtual ~IngameUI ();

    const IngameActivity *GetOwner () const;
    bool IsInputEnabled () const;
    void SetInputEnabled (bool inputEnabled);

    void LoadUI ();
    void SetupUnitsIcons ();
    void ShowMessage (const Urho3D::String &title, const Urho3D::String &description, const Urho3D::String &okButtonText,
                        UICallback callback);
    void ClearUI ();

private:
    struct MessageData
    {
        Urho3D::String title;
        Urho3D::String description;
        Urho3D::String okButtonText;
        UICallback uiCallback;
    };

    void LoadElements ();
    void ShowNextMessage ();

    void SubscribeToEvents ();
    void SubscribeToTopBarEvents ();
    void SubscribeToMenuEvents ();
    void SubscribeToMessageWindowEvents ();

    void HandleTopBarMenuClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleMenuCloseClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleMenuExitToMainClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleMenuExitFromGameClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleMessageWindowOkClicked (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

    IngameActivity *owner_;
    bool inputEnabled_;

    Urho3D::Window *topBar_;
    Urho3D::Window *menu_;
    Urho3D::Window *messageWindow_;
    Urho3D::List <MessageData> requestedMessages_;
};
}
