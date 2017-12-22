#pragma once
#include <ActivitiesApplication/Activity.hpp>

namespace CastlesStrategy
{
class ServerActivity : public ActivitiesApplication::Activity
{
public:
    ServerActivity (Urho3D::Context *context);
    virtual ~ServerActivity ();

    virtual void Start ();
    virtual void Update (float timeStep);
    virtual void Stop ();

private:
};
}
