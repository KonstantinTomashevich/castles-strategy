#include "Unit.hpp"
#include <Urho3D/Core/Context.h>
#include <Utils/UniversalException.hpp>

namespace CastlesStrategy
{
Unit::Unit (Urho3D::Context *context) :
        Urho3D::Component (context),

        belongsToFirst_ (false),
        hp_ (0),
        unitType_ (0),
        attackCooldown_ (0.0f),

        routeIndex_ (0),
        currentWaypointIndex_ (0)
{

}

Unit::~Unit ()
{

}

void Unit::RegisterObjectType (Urho3D::Context *context)
{
    context->RegisterFactory <Unit> ();
    URHO3D_ACCESSOR_ATTRIBUTE ("Is Enabled", IsEnabled, SetEnabled, bool, true, Urho3D::AM_DEFAULT);

    URHO3D_ACCESSOR_ATTRIBUTE ("Is Belongs To First", IsBelongsToFirst, SetBelongsToFirst, bool, false, Urho3D::AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE ("HP", GetHp, SetHp, unsigned int, 0, Urho3D::AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE ("Unit Type", GetUnitType, SetUnitType, unsigned int, 0, Urho3D::AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE ("Attack Cooldown", GetAttackCooldown, SetAttackCooldown, float, 0.0f, Urho3D::AM_DEFAULT);

    URHO3D_ACCESSOR_ATTRIBUTE ("Route Index", GetRouteIndex, SetRouteIndex, unsigned int, 0, Urho3D::AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE ("Current Waypoint Index", GetCurrentWaypointIndex, SetCurrentWaypointIndex, unsigned int, 0, Urho3D::AM_DEFAULT);
}

void Unit::UpdateCooldowns (float timeStep)
{
    attackCooldown_ -= timeStep;
    if (attackCooldown_ < 0.0f)
    {
        attackCooldown_ = 0.0f;
    }
}

bool Unit::IsBelongsToFirst () const
{
    return belongsToFirst_;
}

void Unit::SetBelongsToFirst (bool belongsToFirst)
{
    belongsToFirst_ = belongsToFirst;
    MarkNetworkUpdate ();
}

unsigned int Unit::GetHp () const
{
    return hp_;
}

void Unit::SetHp (unsigned int hp)
{
    hp_ = hp;
    MarkNetworkUpdate ();
}

unsigned int Unit::GetUnitType () const
{
    return unitType_;
}

void Unit::SetUnitType (unsigned int unitType)
{
    unitType_ = unitType;
    MarkNetworkUpdate ();
}

float Unit::GetAttackCooldown () const
{
    return attackCooldown_;
}

void Unit::SetAttackCooldown (float attackCooldown)
{
    if (attackCooldown < 0.0f)
    {
        throw UniversalException <Unit> ("Unit: attack cooldown can not be less than 0!");
    }

    attackCooldown_ = attackCooldown;
    MarkNetworkUpdate ();
}

unsigned int Unit::GetCurrentWaypointIndex () const
{
    return currentWaypointIndex_;
}

void Unit::SetCurrentWaypointIndex (unsigned int currentWaypointIndex)
{
    currentWaypointIndex_ = currentWaypointIndex;
    MarkNetworkUpdate ();
}

unsigned int Unit::GetRouteIndex () const
{
    return routeIndex_;
}

void Unit::SetRouteIndex (unsigned int routeIndex)
{
    routeIndex_ = routeIndex;
    MarkNetworkUpdate ();
}

void Unit::OnSceneSet (Urho3D::Scene *scene)
{
    Urho3D::Component::OnSceneSet (scene);
}
}
