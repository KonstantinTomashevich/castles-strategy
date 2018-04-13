#include "Village.hpp"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>
#include <Utils/UniversalException.hpp>

namespace CastlesStrategy
{
Village::Village (Urho3D::Context *context) : Urho3D::Component (context),
    radius_ (0.1f),
    ownership_ (0.0f),
    wealthLevel_ (1.0f),
    prefabPath_ ()
{

}

Village::~Village ()
{

}

void Village::RegisterObject (Urho3D::Context *context)
{
    context->RegisterFactory <Village> ("Logic");
    URHO3D_ACCESSOR_ATTRIBUTE ("Is Enabled", IsEnabled, SetEnabled, bool, true, Urho3D::AM_DEFAULT);

    URHO3D_ACCESSOR_ATTRIBUTE ("Radius", GetRadius, SetRadius, float, 0.1f, Urho3D::AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE ("Ownership", GetOwnership, SetOwnership, float, 0.0f, Urho3D::AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE ("Wealth Level", GetWealthLevel, SetWealthLevel, float, 1.0f, Urho3D::AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE ("Prefab Path", GetPrefabPath, SetPrefabPath, 
            Urho3D::String, Urho3D::String::EMPTY, Urho3D::AM_DEFAULT);
}

void Village::UpdateOwnership (float timeStep, const UnitsManager *unitsManager)
{
    Urho3D::Vector3 nodePosition = node_->GetPosition ();
    Urho3D::PODVector <const Unit *> unitsNear = unitsManager->GetUnitsNear (
            {nodePosition.x_, nodePosition.z_}, radius_);

    for (const Unit *unit : unitsNear)
    {
        ownership_ += unit->GetHp () * timeStep * (unit->IsBelongsToFirst () ? 1.0f : -1.0f);
    }

    FixOwnership ();
    MarkNetworkUpdate ();
}

unsigned int Village::TakeCoins (float timeStep) const
{
    return Urho3D::Abs (Urho3D::RoundToInt (ownership_ * wealthLevel_ * OWNERSHIP_TO_MONEY_PER_SECOND * timeStep));
}

float Village::GetRadius () const
{
    return radius_;
}

void Village::SetRadius (float radius)
{
    if (radius <= 0.0f)
    {
        throw UniversalException <Village> ("Village: radius can not be zero or negative!");
    }

    radius_ = radius;
    MarkNetworkUpdate ();
}

float Village::GetOwnership () const
{
    return ownership_;
}

void Village::SetOwnership (float ownership)
{
    ownership_ = ownership;
    FixOwnership ();
    MarkNetworkUpdate ();
}

float Village::GetWealthLevel () const
{
    return wealthLevel_;
}

void Village::SetWealthLevel (float wealthLevel)
{
    if (wealthLevel <= 0.0f)
    {
        throw UniversalException <Village> ("Village: wealth level can not be zero or negative!");
    }

    wealthLevel_ = wealthLevel;
    MarkNetworkUpdate ();
}

const Urho3D::String &Village::GetPrefabPath () const
{
    return prefabPath_;
}

void Village::SetPrefabPath (const Urho3D::String &prefabPath)
{
    prefabPath_ = prefabPath;
    MarkNetworkUpdate ();
}

void Village::FixOwnership ()
{
    if (ownership_ > MAX_OWNERSHIP_POINTS)
    {
        ownership_ = MAX_OWNERSHIP_POINTS;
    }
    else if (ownership_ < -MAX_OWNERSHIP_POINTS)
    {
        ownership_ = -MAX_OWNERSHIP_POINTS;
    }
}
}
