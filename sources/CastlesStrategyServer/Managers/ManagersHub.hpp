#pragma once
#include <Urho3D/Container/Vector.h>
#include <CastlesStrategyServer/Managers/Manager.hpp>

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
    ManagersHub ();
    virtual ~ManagersHub ();

    Manager *GetManager (ManagerIndex index);
    const Manager *GetManager (ManagerIndex index) const;
    void HandleUpdate (float timeStep);

private:
    Urho3D::PODVector <Manager *> managers_;
};
}
