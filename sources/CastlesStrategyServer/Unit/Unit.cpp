#include "Unit.hpp"
#include <Urho3D/Core/Context.h>
#include <ActivitiesApplication/UniversalException.hpp>

namespace CastlesStrategy
{
static const Urho3D::StringVector waypointsElementsNames =
{
    "Waypoints Count",
    "   Waypoint"
};

Unit::Unit (Urho3D::Context *context) :
        Urho3D::Component (context),

        owner_ (0),
        hp_ (0),
        unitType_ (0),
        attackCooldown_ (0.0f),

        waypoints_ (),
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

    URHO3D_ACCESSOR_ATTRIBUTE ("Owner", GetOwner, SetOwner, unsigned int, 0, Urho3D::AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE ("HP", GetHp, SetHp, unsigned int, 0, Urho3D::AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE ("Unit Type", GetUnitType, SetUnitType, unsigned int, 0, Urho3D::AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE ("Attack Cooldown", GetAttackCooldown, SetAttackCooldown, float, 0.0f, Urho3D::AM_DEFAULT);

    URHO3D_MIXED_ACCESSOR_ATTRIBUTE ("Waypoints", GetWaypointsAttribute, SetWaypointsAttribute, Urho3D::VariantVector,
        Urho3D::Variant::emptyVariantVector, Urho3D::AM_DEFAULT)
            .SetMetadata (Urho3D::AttributeMetadata::P_VECTOR_STRUCT_ELEMENTS, waypointsElementsNames);
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

unsigned int Unit::GetOwner () const
{
    return owner_;
}

void Unit::SetOwner (unsigned int owner)
{
    owner_ = owner;
}

unsigned int Unit::GetHp () const
{
    return hp_;
}

void Unit::SetHp (unsigned int hp)
{
    hp_ = hp;
}

unsigned int Unit::GetUnitType () const
{
    return unitType_;
}

void Unit::SetUnitType (unsigned int unitType)
{
    unitType_ = unitType;
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
}

const Urho3D::PODVector <Urho3D::Vector2> &Unit::GetWaypoints () const
{
    return waypoints_;
}

void Unit::SetWaypoints (const Urho3D::PODVector <Urho3D::Vector2> &waypoints)
{
    waypoints_ = waypoints;
}

Urho3D::VariantVector Unit::GetWaypointsAttribute () const
{
    Urho3D::VariantVector attribute;
    attribute.Reserve (waypoints_.Size () + 1);
    attribute.Push (waypoints_.Size ());

    for (auto &waypoint : waypoints_)
    {
        attribute.Push (waypoint);
    }

    return attribute;
}

void Unit::SetWaypointsAttribute (const Urho3D::VariantVector &waypoints)
{
    waypoints_.Clear ();
    if (waypoints.Empty ())
    {
        return;
    }

    waypoints_.Reserve (waypoints [0].GetUInt ());
    for (unsigned index = 0; index < waypoints [0].GetUInt (); index++)
    {
        if (index + 1 < waypoints.Size ())
        {
            waypoints_.Push (waypoints [index + 1].GetVector2 ());
        }
        else
        {
            waypoints_.Push (Urho3D::Vector2::ZERO);
        }
    }
}

unsigned int Unit::GetCurrentWaypointIndex () const
{
    return currentWaypointIndex_;
}

void Unit::SetCurrentWaypointIndex (unsigned int currentWaypointIndex)
{
    currentWaypointIndex_ = currentWaypointIndex;
}

void Unit::OnSceneSet (Urho3D::Scene *scene)
{
    Urho3D::Component::OnSceneSet (scene);
}
}
