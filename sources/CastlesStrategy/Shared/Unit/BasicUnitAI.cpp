#include "BasicUnitAI.hpp"
#include <climits>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>

#include <Urho3D/IO/Log.h>
#include <Urho3D/Navigation/NavigationMesh.h>

#include <CastlesStrategy/Server/Managers/UnitsManager.hpp>
#include <CastlesStrategy/Server/Managers/Map.hpp>
#include <CastlesStrategy/Server/Managers/ManagersHub.hpp>

namespace CastlesStrategy
{
UnitCommand BasicUnitAI (Unit *self, const UnitType &unitType, const ManagersHub *managersHub)
{
    const UnitsManager *unitsManager = dynamic_cast <const UnitsManager *> (managersHub->GetManager (MI_UNITS_MANAGER));
    const Unit *nearestEnemy = unitsManager->GetNearestEnemy (self);

    float distance = nearestEnemy != nullptr ?
                     (self->GetNode ()->GetWorldPosition () - nearestEnemy->GetNode ()->GetWorldPosition ()).Length () : 0.0f;

    if (nearestEnemy != nullptr && distance <= unitType.GetAttackRange ())
    {
        return {UCT_ATTACK_UNIT, nearestEnemy->GetID ()};
    }
    else if (nearestEnemy != nullptr && distance <= unitType.GetVisionRange ())
    {
        return {UCT_FOLLOW_UNIT, nearestEnemy->GetID ()};
    }
    else
    {
        const Map *map = dynamic_cast <const Map *> (managersHub->GetManager (MI_MAP));
        Urho3D::Vector2 nextWaypoint = map->GetWaypoint (
                self->GetRouteIndex (), self->GetCurrentWaypointIndex (), self->IsBelongsToFirst ());

        Urho3D::Vector3 target = self->GetScene ()->GetComponent <Urho3D::NavigationMesh> ()->FindNearestPoint (
                {nextWaypoint.x_, 0.0f, nextWaypoint.y_}, Urho3D::Vector3 (1.0f, INT_MAX, 1.0f));

        distance = (self->GetNode ()->GetWorldPosition () - target).Length ();
        unsigned nextWaypointIndex = self->GetCurrentWaypointIndex () + 1;

        if (distance < unitType.GetAttackRange () &&
                nextWaypointIndex < map->GetRoutes () [self->GetRouteIndex ()].GetWaypoints ().Size ())
        {
            self->SetCurrentWaypointIndex (nextWaypointIndex);
        }

        return {UCT_MOVE_TO_WAYPOINT, 0};
    }
}
}
