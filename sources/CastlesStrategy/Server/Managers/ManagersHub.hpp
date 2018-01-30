#pragma once
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Scene/Scene.h>
#include <CastlesStrategy/Server/Managers/Manager.hpp>

namespace CastlesStrategy
{
enum ManagerIndex
{
    MI_UNITS_MANAGER = 0,
    MI_MAP,
    MI_PLAYERS_MANAGER,
    MI_MANAGERS_COUNT
};

class ManagersHub
{
public:
    ManagersHub (Urho3D::Scene *scene);
    virtual ~ManagersHub ();

    Manager *GetManager (ManagerIndex index);
    const Manager *GetManager (ManagerIndex index) const;

    Urho3D::Scene *GetScene () const;
    void HandleUpdate (float timeStep);

private:
    Urho3D::PODVector <Manager *> managers_;
    Urho3D::Scene *scene_;
};
}
