#include "Village.hpp"

namespace CastlesStrategy
{
Village::Village (const Urho3D::Vector2 position, float radius) :
    position_ (position),
    radius_ (radius),
    ownership_ (0)
{

}

Village::~Village ()
{

}

const Urho3D::Vector2 &Village::GetPosition () const
{
    return position_;
}

float Village::GetRadius () const
{
    return radius_;
}

float Village::GetOwnership () const
{
    return ownership_;
}

void Village::UpdateOwnership (float timeStep, const UnitsManager *unitsManager)
{
    Urho3D::PODVector <const Unit *> unitsNear = unitsManager->GetUnitsNear (position_, radius_);
    for (const Unit *unit : unitsNear)
    {
        ownership_ += unit->GetHp () * timeStep * (unit->IsBelongsToFirst () ? 1.0f : -1.0f);
    }
    
    if (ownership_ > MAX_OWNERSHIP_POINTS)
    {
        ownership_ = MAX_OWNERSHIP_POINTS;
    }
    else if (ownership_ < -MAX_OWNERSHIP_POINTS)
    {
        ownership_ = -MAX_OWNERSHIP_POINTS;
    }
}

float Village::TakeMoney (float timeStep) const
{
    return ownership_ * OWNERSHIP_TO_MONEY_PER_SECOND * timeStep;
}
}
