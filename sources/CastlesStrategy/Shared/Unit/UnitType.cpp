#include "UnitType.hpp"
#include <Utils/UniversalException.hpp>

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

UnitType::UnitType (unsigned int id, unsigned int recruitmentCost, float recruitmentTime, float attackRange,
            float attackSpeed, unsigned int attackForce, float visionRange, float navigationRadius, float moveSpeed,
            unsigned int maxHp, const Urho3D::String &prefabPath, const Urho3D::String &iconPath,
            const Urho3D::HashMap <unsigned int, float> &nonDefaultAttackModifiers) :
        id_ (id),
        recruitmentCost_ (recruitmentCost),
        recruitmentTime_ (recruitmentTime),

        attackRange_ (attackRange),
        attackSpeed_ (attackSpeed),
        attackForce_ (attackForce),
        visionRange_ (visionRange),

        navigationRadius_ (navigationRadius),
        moveSpeed_ (moveSpeed),
        maxHp_ (maxHp),

        prefabPath_ (prefabPath),
        iconPath_ (iconPath),
        aiProcessor_ (nullptr),
        nonDefaultAttackModifiers_ (nonDefaultAttackModifiers)
{
    Check ();
}

UnitType::UnitType (const UnitType &another) :
        id_ (another.id_),
        recruitmentCost_ (another.recruitmentCost_),
        recruitmentTime_ (another.recruitmentTime_),

        attackRange_ (another.attackRange_),
        attackSpeed_ (another.attackSpeed_),
        attackForce_ (another.attackForce_),
        visionRange_ (another.visionRange_),

        navigationRadius_ (another.navigationRadius_),
        moveSpeed_ (another.moveSpeed_),
        maxHp_ (another.maxHp_),

        prefabPath_ (another.prefabPath_),
        iconPath_ (another.iconPath_),
        aiProcessor_ (another.aiProcessor_),
        nonDefaultAttackModifiers_ (another.nonDefaultAttackModifiers_)
{

}

UnitType::~UnitType ()
{

}

unsigned int UnitType::GetId () const
{
    return id_;
}

unsigned int UnitType::GetRecruitmentCost () const
{
    return recruitmentCost_;
}

float UnitType::GetRecruitmentTime () const
{
    return recruitmentTime_;
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

const Urho3D::String &UnitType::GetIconPath () const
{
    return iconPath_;
}

float UnitType::GetAttackModiferVersus (unsigned int unitType) const
{
    auto iterator = nonDefaultAttackModifiers_.Find (unitType);
    if (iterator != nonDefaultAttackModifiers_.End ())
    {
        return iterator->second_;
    }
    else
    {
        return 1.0f;
    }
}

void UnitType::SaveToXML (Urho3D::XMLElement &output) const
{
    output.SetUInt ("recruitmentCost", recruitmentCost_);
    output.SetFloat ("recruitmentTime", recruitmentTime_);

    output.SetFloat ("attackRange", attackRange_);
    output.SetFloat ("attackSpeed", attackSpeed_);
    output.SetUInt ("attackForce", attackForce_);
    output.SetFloat ("visionRange", visionRange_);

    output.SetFloat ("navigationRadius", navigationRadius_);
    output.SetFloat ("moveSpeed", moveSpeed_);
    output.SetUInt ("maxHp", maxHp_);

    output.SetAttribute ("prefabPath", prefabPath_);
    output.SetAttribute ("iconPath", iconPath_);

    for (const auto &modifier : nonDefaultAttackModifiers_)
    {
        Urho3D::XMLElement modifierElement = output.CreateChild ("attackModifier");
        modifierElement.SetUInt ("vs", modifier.first_);
        modifierElement.SetFloat ("value", modifier.second_);
    }
}

UnitType UnitType::LoadFromXML (unsigned int id, const Urho3D::XMLElement &input)
{
    Urho3D::HashMap <unsigned int, float> nonDefaultAttackModifiers;
    Urho3D::XMLElement attackModifier = input.GetChild ("attackModifier");

    while (attackModifier.NotNull ())
    {
        nonDefaultAttackModifiers [attackModifier.GetUInt ("vs")] = attackModifier.GetFloat ("value");
        attackModifier = attackModifier.GetNext ("attackModifier");
    }

    return UnitType (id, input.GetUInt ("recruitmentCost"), input.GetFloat ("recruitmentTime"),
            input.GetFloat ("attackRange"), input.GetFloat ("attackSpeed"), input.GetUInt ("attackForce"),
            input.GetFloat ("visionRange"), input.GetFloat ("navigationRadius"), input.GetFloat ("moveSpeed"),
            input.GetUInt ("maxHp"), input.GetAttribute ("prefabPath"), input.GetAttribute ("iconPath"),
            nonDefaultAttackModifiers);
}

void UnitType::Check ()
{
    if (recruitmentTime_ <= 0.0f)
    {
        throw UniversalException <UnitType> ("UnitType: recruitment time must be more than 0!");
    }

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
        throw UniversalException <UnitType> ("UnitType: prefab path must not be more empty!");
    }

    if (iconPath_.Empty ())
    {
        throw UniversalException <UnitType> ("UnitType: icon path must not be more empty!");
    }

    if (prefabPath_.Empty ())
    {
        throw UniversalException <UnitType> ("UnitType: prefab path must not be more empty!");
    }

    for (const auto &modifier : nonDefaultAttackModifiers_)
    {
        if (modifier.second_ < 0.0f)
        {
            throw UniversalException <UnitType> ("UnitType: attack modifier can not be negative, but it is vs " +
                Urho3D::String (modifier.first_) + "!");
        }
    }
}
}
