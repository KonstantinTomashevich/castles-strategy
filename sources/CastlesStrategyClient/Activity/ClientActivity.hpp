#pragma once
#include <ActivitiesApplication/Activity.hpp>

namespace CastlesStrategy
{
class ClientActivity : public ActivitiesApplication::Activity
{
public:
    ClientActivity (Urho3D::Context *context);
    virtual ~ClientActivity ();

    virtual void Start ();
    virtual void Update (float timeStep);
    virtual void Stop ();

private:
};
}
