#pragma once
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Container/List.h>
#include <CastlesStrategy/Server/Managers/ManagersHub.hpp>

namespace CastlesStrategy
{
URHO3D_EVENT (PLAYER_UNITS_PULL_SYNC, PlayerUnitsPullSync)
{
    // Pointer.
    URHO3D_PARAM (PLAYER, Player);
    // UInt.
    URHO3D_PARAM (UNIT_TYPE, UnitType);
    // UInt.
    URHO3D_PARAM (NEW_VALUE, NewValue);
}

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

    unsigned int GetBelongingMaterialIndex () const;
    void SetBelongingMaterialIndex (unsigned int belongingMaterialIndex);

    const Urho3D::List <RecruitmentOrder> &GetOrders () const;
    void AddOrder (unsigned int unitType);
    void RemoveOrder (unsigned int unitType);

    void TakeUnitFromPull (unsigned int unitType);
    unsigned int GetUnitsPullCount (unsigned int unitType) const;
    Player &operator = (const Player &another);

private:
    void SendUnitsPullSyncRequest (unsigned int unitType);

    const ManagersHub *managersHub_;
    unsigned int coins_;
    unsigned int belongingMaterialIndex_;

    Urho3D::List <RecruitmentOrder> orders_;
    Urho3D::PODVector <unsigned int> unitsPull_;
};
}
