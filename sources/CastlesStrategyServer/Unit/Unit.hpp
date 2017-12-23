#pragma once
#include <Urho3D/Scene/Component.h>
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Math/Vector2.h>

namespace CastlesStrategy
{
class Unit : public Urho3D::Component
{
URHO3D_OBJECT (Unit, Component)
public:
    Unit (Urho3D::Context *context);
    virtual ~Unit ();
    static void RegisterObjectType (Urho3D::Context *context);

    unsigned int GetHp () const;
    void SetHp (unsigned int hp);

    unsigned int GetUnitType () const;
    void SetUnitType (unsigned int unitType);

    float GetAttackCooldown () const;
    void SetAttackCooldown (float attackCooldown);

    const Urho3D::PODVector <Urho3D::Vector2> &GetWaypoints () const;
    void SetWaypoints (const Urho3D::PODVector <Urho3D::Vector2> &waypoints);

    Urho3D::VariantVector GetWaypointsAttribute () const;
    void SetWaypointsAttribute (const Urho3D::VariantVector &waypoints);

    unsigned int GetCurrentWaypointIndex () const;
    void SetCurrentWaypointIndex (unsigned int currentWaypointIndex);

protected:
    virtual void OnSceneSet (Urho3D::Scene *scene);

private:
    unsigned int hp_;
    unsigned int unitType_;
    float attackCooldown_;

    Urho3D::PODVector <Urho3D::Vector2> waypoints_;
    unsigned int currentWaypointIndex_;
};
}
