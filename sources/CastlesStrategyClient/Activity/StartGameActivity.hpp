#pragma once
#include <ActivitiesApplication/Activity.hpp>
#include <Utils/UIResizer.hpp>

namespace CastlesStrategy
{
class StartGameActivity : public ActivitiesApplication::Activity
{
public:
    StartGameActivity (Urho3D::Context *context);
    virtual ~StartGameActivity ();

    virtual void Start ();
    virtual void Update (float timeStep);
    virtual void Stop ();

private:
    UIResizer *uiResizer_;
};
}
