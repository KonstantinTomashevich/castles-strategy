#pragma once
#include <Urho3D/Math/Vector2.h>
#include <CastlesStrategy/Server/Managers/UnitsManager.hpp>

namespace CastlesStrategy
{
const float MAX_OWNERSHIP_POINTS = 1000.0f;
const float OWNERSHIP_TO_MONEY_PER_SECOND = 0.1f;

class Village : public Urho3D::Component
{
URHO3D_OBJECT (Village, Component);
public:
    Village (Urho3D::Context *context);
    virtual ~Village ();

    static void RegisterObject (Urho3D::Context *context);
    void UpdateOwnership (float timeStep, const UnitsManager *unitsManager);
    unsigned int TakeCoins (float timeStep) const;

    float GetRadius () const;
    void SetRadius (float radius);

    float GetOwnership () const;
    void SetOwnership (float ownership);

    float GetWealthLevel () const;
    void SetWealthLevel (float wealthLevel);

    const Urho3D::String &GetPrefabPath () const;
    void SetPrefabPath (const Urho3D::String &prefabPath);

private:
    void FixOwnership ();
    
    float radius_;
    float ownership_;
    float wealthLevel_;
    Urho3D::String prefabPath_;
};
}
