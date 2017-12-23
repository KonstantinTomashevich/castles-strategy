#include "UnitType.hpp"
#include <ActivitiesApplication/Exception.hpp>

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

UnitType::UnitType (unsigned int id, float attackRange, float attackSpeed, float visionRange, float moveSpeed,
                    unsigned int maxHp, const Urho3D::String &modelPath) :
        id_ (id),
        attackRange_ (attackRange),
        attackSpeed_ (attackSpeed),
        visionRange_ (visionRange),
        moveSpeed_ (moveSpeed),
        maxHp_ (maxHp),
        modelPath_ (modelPath)
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

float UnitType::GetVisionRange () const
{
    return visionRange_;
}

float UnitType::GetMoveSpeed () const
{
    return moveSpeed_;
}

unsigned int UnitType::GetMaxHp () const
{
    return maxHp_;
}

const Urho3D::String &UnitType::GetModelPath () const
{
    return modelPath_;
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
    output.SetFloat ("visionRange", visionRange_);

    output.SetFloat ("moveSpeed", moveSpeed_);
    output.SetUInt ("maxHp", maxHp_);
    output.SetAttribute ("modelPath", modelPath_);
}

UnitType *UnitType::LoadFromXML (const Urho3D::XMLElement &input)
{
    return new UnitType (
            input.GetUInt ("id"),
            input.GetFloat ("attackRange"),
            input.GetFloat ("attackSpeed"),
            input.GetFloat ("visionRange"),

            input.GetFloat ("moveSpeed"),
            input.GetUInt ("maxHp"),
            input.GetAttribute ("modelPath")
    );
}

void UnitType::Check ()
{
    if (attackRange_ <= 0.0f)
    {
        throw Exception <UnitType> ("UnitType: attack range must be more than 0!");
    }

    if (attackSpeed_ <= 0.0f)
    {
        throw Exception <UnitType> ("UnitType: attack speed must be more than 0!");
    }

    if (visionRange_ <= 0.0f)
    {
        throw Exception <UnitType> ("UnitType: vision range must be more than 0!");
    }

    if (moveSpeed_ <= 0.0f)
    {
        throw Exception <UnitType> ("UnitType: attack range must be more than 0!");
    }

    if (maxHp_ == 0)
    {
        throw Exception <UnitType> ("UnitType: max HP must be more than 0!");
    }

    if (modelPath_.Empty ())
    {
        throw Exception <UnitType> ("UnitType: model path must not be more empty!");
    }
}
}
