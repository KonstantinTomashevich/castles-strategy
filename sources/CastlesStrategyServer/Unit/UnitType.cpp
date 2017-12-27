#include "UnitType.hpp"
#include <ActivitiesApplication/UniversalException.hpp>

namespace CastlesStrategy
{

UnitCommand::UnitCommand (UnitCommandType commandType, unsigned int argument) :
        commandType_ (commandType),
        argument_ (argument)
{}

UnitCommand::~UnitCommand ()
{

}

bool UnitCommand::operator == (const UnitCommand &rhs) const
{
    return commandType_ == rhs.commandType_ &&
           argument_ == rhs.argument_;
}

bool UnitCommand::operator != (const UnitCommand &rhs) const
{
    return !(rhs == *this);
}

UnitType::UnitType (unsigned int id, float attackRange, float attackSpeed, unsigned int attackForce,
                    float visionRange, float navigationRadius, float moveSpeed, unsigned int maxHp,
                    const Urho3D::String &prefabPath) :
        id_ (id),
        attackRange_ (attackRange),
        attackSpeed_ (attackSpeed),
        attackForce_ (attackForce),
        visionRange_ (visionRange),
        navigationRadius_ (navigationRadius),
        moveSpeed_ (moveSpeed),
        maxHp_ (maxHp),
        prefabPath_ (prefabPath)
{
    Check ();
}

UnitType::~UnitType ()
{

}

unsigned int UnitType::GetId () const
{
    return id_;
}

float UnitType::GetAttackRange () const
{
    return attackRange_;
}

float UnitType::GetAttackSpeed () const
{
    return attackSpeed_;
}

unsigned int UnitType::GetAttackForce () const
{
    return attackForce_;
}

float UnitType::GetVisionRange () const
{
    return visionRange_;
}

float UnitType::GetNavigationRadius () const
{
    return navigationRadius_;
}

float UnitType::GetMoveSpeed () const
{
    return moveSpeed_;
}

unsigned int UnitType::GetMaxHp () const
{
    return maxHp_;
}

const Urho3D::String &UnitType::GetPrefabPath () const
{
    return prefabPath_;
}

UnitAIProcessor UnitType::GetAiProcessor () const
{
    return aiProcessor_;
}

void UnitType::SetAiProcessor (UnitAIProcessor aiProcessor)
{
    aiProcessor_ = aiProcessor;
}

void UnitType::SaveToXML (Urho3D::XMLElement &output) const
{
    output.SetUInt ("id", id_);
    output.SetFloat ("attackRange", attackRange_);
    output.SetFloat ("attackSpeed", attackSpeed_);
    output.SetUInt ("attackForce", attackForce_);
    output.SetFloat ("visionRange", visionRange_);

    output.SetFloat ("navigationRadius", navigationRadius_);
    output.SetFloat ("moveSpeed", moveSpeed_);
    output.SetUInt ("maxHp", maxHp_);
    output.SetAttribute ("prefabPath", prefabPath_);
}

UnitType UnitType::LoadFromXML (const Urho3D::XMLElement &input)
{
    return UnitType (input.GetUInt ("id"), input.GetFloat ("attackRange"), input.GetFloat ("attackSpeed"),
                     input.GetUInt ("attackForce"), input.GetFloat ("visionRange"), input.GetFloat ("navigationRadius"),
                     input.GetFloat ("moveSpeed"), input.GetUInt ("maxHp"), input.GetAttribute ("prefabPath"));
}

void UnitType::Check ()
{
    if (attackRange_ <= 0.0f)
    {
        throw UniversalException <UnitType> ("UnitType: attack range must be more than 0!");
    }

    if (attackSpeed_ <= 0.0f)
    {
        throw UniversalException <UnitType> ("UnitType: attack speed must be more than 0!");
    }

    if (visionRange_ <= 0.0f)
    {
        throw UniversalException <UnitType> ("UnitType: vision range must be more than 0!");
    }

    if (navigationRadius_ <= 0.0f)
    {
        throw UniversalException <UnitType> ("UnitType: navigation radius must be more than 0!");
    }

    if (moveSpeed_ <= 0.0f)
    {
        throw UniversalException <UnitType> ("UnitType: move speed must be more than 0!");
    }

    if (maxHp_ == 0)
    {
        throw UniversalException <UnitType> ("UnitType: max HP must be more than 0!");
    }

    if (prefabPath_.Empty ())
    {
        throw UniversalException <UnitType> ("UnitType: model path must not be more empty!");
    }
}
}
