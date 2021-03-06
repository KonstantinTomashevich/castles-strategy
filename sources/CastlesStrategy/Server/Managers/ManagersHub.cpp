#include "ManagersHub.hpp"
#include <CastlesStrategy/Server/Managers/UnitsManager.hpp>
#include <CastlesStrategy/Server/Managers/Map.hpp>
#include <CastlesStrategy/Server/Managers/PlayersManager.hpp>
#include <CastlesStrategy/Server/Managers/VillagesManager.hpp>
#include <Utils/UniversalException.hpp>

namespace CastlesStrategy
{
ManagersHub::ManagersHub (Urho3D::Scene *scene) :
        managers_ (MI_MANAGERS_COUNT),
        scene_ (scene)
{
    managers_ [MI_UNITS_MANAGER] = new UnitsManager (this);
    managers_ [MI_MAP] = new Map (this);
    managers_ [MI_PLAYERS_MANAGER] = new PlayersManager (this);
    managers_ [MI_VILLAGES_MANAGER] = new VillagesManager (this);
}

ManagersHub::~ManagersHub ()
{
    for (Manager *&manager : managers_)
    {
        delete manager;
    }
}

Manager *ManagersHub::GetManager (ManagerIndex index)
{
    if (index == MI_MANAGERS_COUNT)
    {
        throw UniversalException <ManagersHub> ("ManagersHub: manager index is out of range!");
    }

    return managers_ [index];
}

const Manager * ManagersHub::GetManager (ManagerIndex index) const
{
    if (index == MI_MANAGERS_COUNT)
    {
        throw UniversalException <ManagersHub> ("ManagersHub: manager index is out of range!");
    }

    return managers_ [index];
}

Urho3D::Scene *ManagersHub::GetScene () const
{
    return scene_;
}

void ManagersHub::HandleUpdate (float timeStep)
{
    if (scene_ != nullptr)
    {
        scene_->Update (timeStep);
    }

    for (Manager *&manager : managers_)
    {
        manager->HandleUpdate (timeStep);
    }
}
}
