#include "BasicUnitAi.hpp"
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>

#include <Urho3D/IO/Log.h>
#include <Urho3D/Navigation/NavigationMesh.h>
#include <CastlesStrategyServer/Managers/UnitsManager.hpp>

namespace CastlesStrategy
{
UnitCommand BasicUnitAI (Unit *self, const UnitType &unitType, UnitsManager *unitsManager)
{
    Unit *nearestEnemy = unitsManager->GetNearestEnemy (self);
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
        Urho3D::Vector3 target = self->GetWaypoints () [self->GetCurrentWaypointIndex ()];
        target.z_ = target.y_;
        target.y_ = 0.0f;

        target = self->GetScene ()->GetComponent <Urho3D::NavigationMesh> ()->FindNearestPoint (target,
            Urho3D::Vector3 (1.0f, INT_MAX, 1.0f));

        distance = (self->GetNode ()->GetWorldPosition () - target).Length ();
        unsigned nextWaypoint = self->GetCurrentWaypointIndex () + 1;


        if (distance < unitType.GetAttackRange () && nextWaypoint < self->GetWaypoints ().Size ())
        {
            self->SetCurrentWaypointIndex (nextWaypoint);
        }

        return {UCT_MOVE_TO_WAYPOINT, 0};
    }
}
}
