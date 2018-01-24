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
};
