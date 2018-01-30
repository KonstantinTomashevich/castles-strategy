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
    void UpdateCooldowns (float timeStep);

    bool IsBelongsToFirst () const;
    void SetBelongsToFirst (bool belongsToFirst);

    unsigned int GetHp () const;
    void SetHp (unsigned int hp);

    unsigned int GetUnitType () const;
    void SetUnitType (unsigned int unitType);

    float GetAttackCooldown () const;
    void SetAttackCooldown (float attackCooldown);

    unsigned int GetRouteIndex () const;
    void SetRouteIndex (unsigned int routeIndex);

    unsigned int GetCurrentWaypointIndex () const;
    void SetCurrentWaypointIndex (unsigned int currentWaypointIndex);

protected:
    virtual void OnSceneSet (Urho3D::Scene *scene);

private:
    bool belongsToFirst_;
    unsigned int hp_;
    unsigned int unitType_;
    float attackCooldown_;

    unsigned int routeIndex_;
    unsigned int currentWaypointIndex_;
};
}
