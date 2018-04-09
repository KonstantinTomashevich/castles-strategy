#pragma once
#include <Urho3D/Math/Vector2.h>
#include <CastlesStrategy/Server/Managers/UnitsManager.hpp>

namespace CastlesStrategy
{
const float MAX_OWNERSHIP_POINTS = 1000.0f;
const float OWNERSHIP_TO_MONEY_PER_SECOND = 0.1f;

class Village
{
public:
    Village (const Urho3D::Vector2 position, float radius);
    virtual ~Village ();

    const Urho3D::Vector2 &GetPosition () const;
    float GetRadius () const;
    float GetOwnership () const;

    void UpdateOwnership (float timeStep, const UnitsManager *unitsManager);
    float TakeMoney (float timeStep) const;

private:
    Urho3D::Vector2 position_;
    float radius_;
    float ownership_;
};
}
