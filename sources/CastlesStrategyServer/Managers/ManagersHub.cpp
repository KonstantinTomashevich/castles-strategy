#include "ManagersHub.hpp"
#include <CastlesStrategyServer/Managers/UnitsManager.hpp>
#include <CastlesStrategyServer/Managers/Map.hpp>
#include <CastlesStrategyServer/Managers/PlayersManager.hpp>
#include <ActivitiesApplication/UniversalException.hpp>

namespace CastlesStrategy
{
ManagersHub::ManagersHub () :
        managers_ (MI_MANAGERS_COUNT)
{
    managers_ [MI_UNITS_MANAGER] = new UnitsManager (this);
    managers_ [MI_MAP] = new Map (this);
    managers_ [MI_PLAYERS_MANAGER] = new PlayersManager (this);
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

void ManagersHub::HandleUpdate (float timeStep)
{
    for (Manager *&manager : managers_)
    {
        manager->HandleUpdate (timeStep);
    }
}
}
