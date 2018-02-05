#pragma once
#include <ActivitiesApplication/ActivitiesApplication.hpp>

class LauncherApplication : public ActivitiesApplication::ActivitiesApplication
{
public:
    explicit LauncherApplication (Urho3D::Context *context);
    virtual ~LauncherApplication ();

    virtual void Setup ();
    virtual void Start ();
    virtual void Stop ();

private:
    void SubscribeToEvents ();
    void HandleShutdownAllActivities (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleStartMainMenu (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleStartClient (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
    void HandleStartServer (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData);
};
