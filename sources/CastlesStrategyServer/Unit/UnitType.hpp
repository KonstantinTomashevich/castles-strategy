#pragma once
#include <Urho3D/Container/Str.h>
#include <Urho3D/Resource/XMLElement.h>
#include <CastlesStrategyServer/Unit/Unit.hpp>

namespace CastlesStrategy
{
class UnitsManager;
class UnitType;

const Urho3D::StringHash UNIT_PREFAB_VAR_HASH ("UnitPrefab");
enum UnitCommandType
{
    UCT_MOVE_TO_WAYPOINT = 0,
    UCT_FOLLOW_UNIT,
    UCT_ATTACK_UNIT
};

struct UnitCommand
{
    UnitCommandType commandType_;
    unsigned argument_;

    UnitCommand (UnitCommandType commandType, unsigned int argument);
    virtual ~UnitCommand ();

    bool operator == (const UnitCommand &rhs) const;
    bool operator != (const UnitCommand &rhs) const;
};

typedef UnitCommand (*UnitAIProcessor) (Unit *self, const UnitType &unitType, UnitsManager *unitsManager);
class UnitType
{
public:
    UnitType (unsigned int id, float attackRange, float attackSpeed, unsigned int attackForce,
                  float visionRange, float navigationRadius, float moveSpeed, unsigned int maxHp,
                  const Urho3D::String &prefabPath);
    virtual ~UnitType ();

    unsigned int GetId () const;
    float GetAttackRange () const;
    float GetAttackSpeed () const;
    unsigned int GetAttackForce () const;
    float GetVisionRange () const;

    float GetNavigationRadius () const;
    float GetMoveSpeed () const;
    unsigned int GetMaxHp () const;
    const Urho3D::String &GetPrefabPath () const;

    UnitAIProcessor GetAiProcessor () const;
    void SetAiProcessor (UnitAIProcessor aiProcessor);

    void SaveToXML (Urho3D::XMLElement &output) const;
    static UnitType LoadFromXML (const Urho3D::XMLElement &input);

private:
    void Check ();

    unsigned int id_;
    float attackRange_;
    float attackSpeed_;
    unsigned int attackForce_;
    float visionRange_;

    float navigationRadius_;
    float moveSpeed_;
    unsigned int maxHp_;
    Urho3D::String prefabPath_;
    UnitAIProcessor aiProcessor_;
};
}
