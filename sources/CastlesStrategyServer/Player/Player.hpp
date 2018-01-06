#pragma once
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Container/List.h>
#include <Urho3D/Container/Str.h>
#include <CastlesStrategyServer/Managers/ManagersHub.hpp>

namespace CastlesStrategy
{
struct RecruitmentOrder
{
    unsigned int unitType_;
    float timeLeft_;
};

class Player
{
public:
    explicit Player (const ManagersHub *managersHub);
    Player (const Player &another);
    virtual ~Player ();

    void HandleUpdate (float timeStep);
    unsigned int GetCoins () const;
    void SetCoins (unsigned int coins);

    const Urho3D::String &GetName () const;
    void SetName (const Urho3D::String &name);

    unsigned int GetBelongingMaterialIndex () const;
    void SetBelongingMaterialIndex (unsigned int belongingMaterialIndex);

    const Urho3D::List <RecruitmentOrder> &GetOrders () const;
    void AddOrder (unsigned int unitType);
    void RemoveOrder (unsigned int unitType);

    void TakeUnitFromPull (unsigned int unitType);
    unsigned int GetUnitsPullCount (unsigned int unitType) const;

private:
    const ManagersHub *managersHub_;
    unsigned int coins_;
    Urho3D::String name_;
    unsigned int belongingMaterialIndex_;

    Urho3D::List <RecruitmentOrder> orders_;
    Urho3D::Vector <unsigned int> unitsPull_;
};
}
