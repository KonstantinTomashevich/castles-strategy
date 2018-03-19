#include "Player.hpp"
#include <CastlesStrategy/Server/Managers/UnitsManager.hpp>
#include <Utils/UniversalException.hpp>

namespace CastlesStrategy
{
Player::Player (const ManagersHub *managersHub) :
    managersHub_ (managersHub),
    coins_ (0),

    orders_ (),
    unitsPull_ (dynamic_cast <const UnitsManager *> (managersHub->GetManager (MI_UNITS_MANAGER))->GetUnitsTypesCount ())
{
    for (unsigned int index = 0; index < unitsPull_.Size (); index++)
    {
        unitsPull_ [index] = 0;
    }
}

Player::Player (const Player &another) : Player (another.managersHub_)
{
    *this = another;
}

Player::~Player ()
{

}

void Player::HandleUpdate (float timeStep)
{
    if (!orders_.Empty ())
    {
        RecruitmentOrder &order = orders_.Front ();
        order.timeLeft_ -= timeStep;

        if (order.timeLeft_ <= 0.0f)
        {
            unitsPull_[order.unitType_]++;
            SendUnitsPullSyncRequest (order.unitType_);
            orders_.PopFront ();
        }
    }
}

unsigned int Player::GetCoins () const
{
    return coins_;
}

void Player::SetCoins (unsigned int coins)
{
    coins_ = coins;
    Urho3D::VariantMap eventData;
    eventData [PlayerCoinsSync::PLAYER] = this;
    eventData [PlayerCoinsSync::NEW_VALUE] = coins;
    managersHub_->GetScene ()->SendEvent (E_PLAYER_COINS_SYNC, eventData);
}

const Urho3D::List <RecruitmentOrder> &Player::GetOrders () const
{
    return orders_;
}

void Player::AddOrder (unsigned int unitType)
{
    const UnitsManager *unitsManager = dynamic_cast <const UnitsManager *> (managersHub_->GetManager (MI_UNITS_MANAGER));
    const UnitType &unitTypeData = unitsManager->GetUnitType (unitType);

    if (coins_ >= unitTypeData.GetRecruitmentCost ())
    {
        SetCoins (coins_ - unitTypeData.GetRecruitmentCost ());
        orders_.Push ({unitType, unitTypeData.GetRecruitmentTime ()});
    }
    else
    {
        throw UniversalException <Player> ("Player: unit type " + Urho3D::String (unitType) +
                " is too expensive to recruit, it costs " + Urho3D::String (unitTypeData.GetRecruitmentCost ()) +
                " coins, but there is only " + Urho3D::String (coins_) + " coins!");
    }
}

void Player::RemoveOrder (unsigned int unitType)
{
    if (orders_.Empty ())
    {
        return;
    }

    const UnitsManager *unitsManager = dynamic_cast <const UnitsManager *> (managersHub_->GetManager (MI_UNITS_MANAGER));
    const UnitType &unitTypeData = unitsManager->GetUnitType (unitType);

    for (auto iterator = ++orders_.Begin (); iterator != orders_.End (); iterator++)
    {
        if (iterator->unitType_ == unitType)
        {
            iterator = orders_.Erase (iterator);
            SetCoins (coins_ + unitTypeData.GetRecruitmentCost ());
            return;
        }
    }

    if (orders_.Front ().unitType_ == unitType)
    {
        SetCoins (coins_ + unitTypeData.GetRecruitmentCost ());
        orders_.PopFront ();
    }
}

void Player::TakeUnitFromPull (unsigned int unitType)
{
    if (GetUnitsPullCount (unitType) == 0)
    {
        throw UniversalException <Player> ("Player: unit with type " + Urho3D::String (unitType) +
                " requested from pull, but there is no units of this type in the pull!");
    }

    unitsPull_ [unitType]--;
    SendUnitsPullSyncRequest (unitType);
}

unsigned int Player::GetUnitsPullCount (unsigned int unitType) const
{
    if (unitType >= unitsPull_.Size ())
    {
        throw UniversalException <Player> ("Player: unit type " + Urho3D::String (unitType) +
                " requested, but there is only " + Urho3D::String (unitsPull_.Size ()) + " unit types!");
    }
    return unitsPull_ [unitType];
}

Player &Player::operator = (const Player &another)
{
    coins_ = another.coins_;
    orders_ = another.orders_;
    unitsPull_ = another.unitsPull_;
}

void Player::SendUnitsPullSyncRequest (unsigned int unitType)
{
    Urho3D::VariantMap eventData;
    eventData [PlayerUnitsPullSync::PLAYER] = this;
    eventData [PlayerUnitsPullSync::UNIT_TYPE] = unitType;
    eventData [PlayerUnitsPullSync::NEW_VALUE] = unitsPull_ [unitType];
    managersHub_->GetScene ()->SendEvent (E_PLAYER_UNITS_PULL_SYNC, eventData);
}
}
