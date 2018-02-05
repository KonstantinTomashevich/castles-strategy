#pragma once
#include <ActivitiesApplication/Activity.hpp>
#include <Utils/UIResizer.hpp>

namespace CastlesStrategy
{
class MainMenuActivity : public ActivitiesApplication::Activity
{
URHO3D_OBJECT (MainMenuActivity, Activity)
public:
    MainMenuActivity (Urho3D::Context *context);
    virtual ~MainMenuActivity ();

    virtual void Start ();
    virtual void Update (float timeStep);
    virtual void Stop ();

private:
    void LoadUILayout ();
    void SubscribeToEvents ();

    void HandleConnectToServerClick (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleStartServerClick (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);

    UIResizer *uiResizer_;
};
}
