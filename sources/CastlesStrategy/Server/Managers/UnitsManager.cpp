#include "UnitsManager.hpp"
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>

#include <Urho3D/Navigation/CrowdAgent.h>
#include <Urho3D/Navigation/NavigationMesh.h>
#include <Urho3D/IO/Log.h>

#include <CastlesStrategy/Shared/Unit/BasicUnitAI.hpp>
#include <CastlesStrategy/Server/Managers/Map.hpp>
#include <CastlesStrategy/Server/Managers/ManagersHub.hpp>

#include <Utils/UniversalException.hpp>
#include <climits>

namespace CastlesStrategy
{
void ProcessUnitCommandMoveOrFollow (
        UnitsManager *unitsManager, Unit *unit, const UnitCommand &command, const UnitType &unitType);

void ProcessUnitCommandAttackUnit (
        UnitsManager *unitsManager, Unit *unit, const UnitCommand &command, const UnitType &unitType);

UnitsManager::UnitsManager (ManagersHub *managersHub) : Manager (managersHub),
    spawnsUnitType_ (0),
    units_ (),
    unitsTypes_ (),
    unitCommandProcessors_ (UCT_COMMANDS_COUNT)
{
    unitCommandProcessors_ [UCT_FOLLOW_UNIT] = ProcessUnitCommandMoveOrFollow;
    unitCommandProcessors_ [UCT_MOVE_TO_WAYPOINT] = ProcessUnitCommandMoveOrFollow;
    unitCommandProcessors_ [UCT_ATTACK_UNIT] = ProcessUnitCommandAttackUnit;
}

UnitsManager::~UnitsManager ()
{

}

void UnitsManager::AddUnit (Unit *unit)
{
    if (!units_.Empty () && units_.Back ()->GetID () >= unit->GetID ())
    {
        throw UniversalException <UnitsManager> ("UnitsManager: attempt to add the same unit twice!");
    }

    units_.Push (unit);
    SetupUnit (unit);
}

const Unit *UnitsManager::GetSpawn (unsigned route, bool belongsToFirst) const
{
    for (const Unit *unit : units_)
    {
        if (unit->GetUnitType () == spawnsUnitType_ && unit->GetRouteIndex () == route &&
            unit->IsBelongsToFirst () == belongsToFirst)
        {
            return unit;
        }
    }
    return nullptr;
}

const Unit *UnitsManager::SpawnUnit (unsigned route, bool belongsToFirst, unsigned unitType)
{
    const Unit *spawn = GetSpawn (route, belongsToFirst);
    if (spawn == nullptr)
    {
        throw UniversalException <UnitsManager> ("UnitsManager: can not find spawn for route " +
                                                 Urho3D::String (route) + " for player " + Urho3D::String (belongsToFirst ? 0 : 1) + "!");
    }

    return SpawnUnit (spawn, unitType);
}

const Unit *UnitsManager::SpawnUnit (unsigned spawnId, unsigned unitType)
{
    const Unit *spawn = GetUnit (spawnId);
    if (spawn == nullptr)
    {
        throw UniversalException <UnitsManager> ("UnitsManager: spawn with id " +
                Urho3D::String (spawnId) + " does not exists!");
    }

    if (spawn->GetUnitType () != spawnsUnitType_)
    {
        throw UniversalException <UnitsManager> ("UnitsManager: unit with id " +
                Urho3D::String (spawnId) + " is not a spawn!");
    }

    return SpawnUnit (spawn, unitType);
}

const Unit *UnitsManager::GetUnit (unsigned int id) const
{
    bool found;
    unsigned index = GetUnitIndex (id, found);
    return found ? units_ [index] : nullptr;
}

Unit *UnitsManager::GetUnit (unsigned int id)
{
    bool found;
    unsigned index = GetUnitIndex (id, found);
    return found ? units_ [index] : nullptr;
}

const Unit *UnitsManager::GetNearestEnemy (Unit *unit) const
{
    float minimumDistance = INT_MAX;
    const Unit *nearestEnemy = nullptr;

    for (const Unit *scanningUnit : units_)
    {
        if (unit->IsBelongsToFirst () != scanningUnit->IsBelongsToFirst ())
        {
            float distance = (unit->GetNode ()->GetWorldPosition () - scanningUnit->GetNode ()->GetWorldPosition ()).Length ();
            if (distance < minimumDistance)
            {
                minimumDistance = distance;
                nearestEnemy = scanningUnit;
            }
        }
    }

    return nearestEnemy;
}

void UnitsManager::HandleUpdate (float timeStep)
{
    ProcessUnits (timeStep);
    ClearDeadUnits ();
}

unsigned int UnitsManager::GetUnitsTypesCount () const
{
    return unitsTypes_.size ();
}

const UnitType &UnitsManager::GetUnitType (unsigned int index) const
{
    if (index >= unitsTypes_.size ())
    {
        throw UniversalException <UnitsManager> ("UnitsManager: unit type " + Urho3D::String (index) +
            " requested, but there is only " + Urho3D::String (unitsTypes_.size ()) + " unit types!");
    }

    return  unitsTypes_ [index];
}

unsigned int UnitsManager::GetSpawnsUnitType () const
{
    return spawnsUnitType_;
}

void UnitsManager::SaveUnitsTypesToXML (Urho3D::XMLElement &output) const
{
    output.SetUInt ("spawnsUnitType", spawnsUnitType_);
    for (const UnitType &unitType : unitsTypes_)
    {
        Urho3D::XMLElement newChild = output.CreateChild ("unitType");
        unitType.SaveToXML (newChild);
    }
}

void UnitsManager::LoadUnitsTypesFromXML (const Urho3D::XMLElement &input)
{
    spawnsUnitType_ = input.GetUInt ("spawnsUnitType");
    Urho3D::XMLElement element = input.GetChild ("unitType");
    unsigned int id = 0;

    while (element.NotNull ())
    {
        // TODO: Currently basic ai setted as default for all units types.
        unitsTypes_.push_back (UnitType::LoadFromXML (id, element));
        unitsTypes_.back ().SetAiProcessor (BasicUnitAI);

        element = element.GetNext ("unitType");
        id++;
    }
}

void UnitsManager::SaveSpawnsToXML (Urho3D::XMLElement &output) const
{
    for (const Unit *unit : units_)
    {
        if (unit->GetUnitType () == spawnsUnitType_)
        {
            Urho3D::XMLElement newChild = output.CreateChild ("spawn");
            Urho3D::Vector3 worldPosition = unit->GetNode ()->GetWorldPosition ();

            newChild.SetVector2 ("position", {worldPosition.x_, worldPosition.z_});
            newChild.SetBool ("belongsToFirst", unit->IsBelongsToFirst ());
            newChild.SetUInt ("route", unit->GetRouteIndex ());
        }
    }
}

void UnitsManager::LoadSpawnsFromXML (const Urho3D::XMLElement &input)
{
    Urho3D::XMLElement element = input.GetChild ("spawn");
    while (element.NotNull ())
    {
        Unit *unit = CreateUnit (element.GetVector2 ("position"), spawnsUnitType_,
                                 element.GetBool ("belongsToFirst"), element.GetUInt ("route"));
        AddUnit (unit);

        unit->GetNode ()->GetComponent <Urho3D::CrowdAgent> ()->SetUpdateNodePosition (false);
        element = element.GetNext ("spawn");
    }
}

const Unit *UnitsManager::SpawnUnit (const Unit *spawn, unsigned unitType)
{
    Urho3D::Vector3 spawnWorldPosition = spawn->GetNode ()->GetWorldPosition ();
    float deltaX = unitsTypes_ [spawnsUnitType_].GetAttackRange ();
    float deltaZ = deltaX;

    deltaX = Urho3D::Random (unitsTypes_ [spawnsUnitType_].GetNavigationRadius (), deltaX);
    deltaZ = Urho3D::Random (unitsTypes_ [spawnsUnitType_].GetNavigationRadius (), deltaZ);

    spawnWorldPosition.x_ += (Urho3D::Random (2) * 2 - 1) * deltaX;
    spawnWorldPosition.z_ += (Urho3D::Random (2) * 2 - 1) * deltaZ;

    Unit *unit = CreateUnit ({spawnWorldPosition.x_, spawnWorldPosition.z_}, unitType,
                             spawn->IsBelongsToFirst (), spawn->GetRouteIndex ());

    AddUnit (unit);
    return unit;
}

unsigned UnitsManager::GetUnitIndex (unsigned id, bool &found) const
{
    unsigned int left = 0;
    unsigned int right = units_.Size () - 1;

    while (left <= right)
    {
        unsigned int medium = left + (right - left) / 2;
        Unit *unit = units_ [medium];

        if (unit->GetID () == id)
        {
            found = true;
            return medium;
        }
        else if (unit->GetID () > id)
        {
            right = medium - 1;
        }
        else
        {
            left = medium + 1;
        }
    }

    found = false;
    return 0;
}

void UnitsManager::ProcessUnits (float timeStep)
{
    for (unsigned index = 0; index < units_.Size (); index++)
    {
        Unit *unit = units_ [index];
        if (unit->GetHp () > 0)
        {
            unit->UpdateCooldowns (timeStep);

            if (unit->GetUnitType () >= unitsTypes_.size ())
            {
                throw UniversalException <UnitsManager> (
                        "UnitsManager: there is only " + Urho3D::String (unitsTypes_.size ()) +
                        " units types, but T" + Urho3D::String (unit->GetUnitType ()) + " requested!");
            }

            const UnitType &unitType = unitsTypes_ [unit->GetUnitType ()];
            UnitCommand command = unitType.GetAiProcessor () (unit, unitType, GetManagersHub ());
            ProcessUnitCommand (unit, command, unitType);
        }
    }
}

void UnitsManager::ClearDeadUnits ()
{
    unsigned offset = 0;
    for (unsigned index = 0; index < units_.Size (); index++)
    {
        if (units_ [index]->GetHp () == 0)
        {
            MakeUnitDead (units_ [index]);
            offset++;
        }
        else if (offset > 0)
        {
            units_ [index - offset] = units_ [index];
        }
    }
    units_.Resize (units_.Size () - offset);
}

Unit *UnitsManager::CreateUnit (Urho3D::Vector2 position, unsigned unitType, bool belongsToFirst, unsigned route)
{
    Urho3D::Node *unitsNode = GetManagersHub ()->GetScene ()->GetChild ("units");
    if (unitsNode == nullptr)
    {
        unitsNode = GetManagersHub ()->GetScene ()->CreateChild ("units", Urho3D::REPLICATED);
    }

    Urho3D::NavigationMesh *navigationMesh = GetManagersHub ()->GetScene ()->GetComponent <Urho3D::NavigationMesh> ();
    Urho3D::Node *unitNode = unitsNode->CreateChild (Urho3D::String::EMPTY, Urho3D::REPLICATED);
    unitNode->SetWorldPosition (
            navigationMesh->FindNearestPoint ({position.x_, 0.0f, position.y_}, Urho3D::Vector3::UP * INT_MAX));

    Unit *unit = unitNode->CreateComponent <Unit> (Urho3D::REPLICATED);
    unit->SetUnitType (unitType);
    unit->SetBelongsToFirst (belongsToFirst);
    unit->SetRouteIndex (route);
    return unit;
}

void UnitsManager::SetupUnit (Unit *unit)
{
    const UnitType &unitType = GetUnitType (unit->GetUnitType ());
    unit->GetNode ()->SetVar (UNIT_PREFAB_VAR_HASH, unitType.GetPrefabPath ());
    Urho3D::CrowdAgent *crowdAgent = unit->GetNode ()->CreateComponent <Urho3D::CrowdAgent> (Urho3D::LOCAL);

    crowdAgent->SetMaxSpeed (unitType.GetMoveSpeed ());
    crowdAgent->SetMaxAccel (INT_MAX);
    crowdAgent->SetRadius (unitType.GetNavigationRadius ());

    unit->SetHp (unitType.GetMaxHp ());
    unit->SetAttackCooldown (0.0f);
    unit->SetCurrentWaypointIndex (0);
}

void UnitsManager::ProcessUnitCommand (Unit *unit, const UnitCommand &command, const UnitType &unitType)
{
    unitCommandProcessors_ [command.commandType_] (this, unit, command, unitType);
}

void UnitsManager::MakeUnitDead (Unit *unit)
{
    if (unit->GetUnitType () == spawnsUnitType_)
    {
        Urho3D::VariantMap eventData;
        eventData [GameEnded::FIRST_WON] = !unit->IsBelongsToFirst ();
        GetManagersHub ()->GetScene ()->SendEvent (E_GAME_ENDED, eventData);
    }

    // TODO: Temporary (reimplement).
    unit->GetNode ()->Remove ();
}

void ProcessUnitCommandMoveOrFollow (UnitsManager *unitsManager, Unit *unit, const UnitCommand &command,
                                     const UnitType &unitType)
{
    Urho3D::Vector3 target;
    if (command.commandType_ == UCT_MOVE_TO_WAYPOINT)
    {
        const Map *map = dynamic_cast <const Map *> (unitsManager->GetManagersHub ()->GetManager (MI_MAP));
        Urho3D::Vector2 nextWaypoint = map->GetWaypoint (
                unit->GetRouteIndex (), unit->GetCurrentWaypointIndex (), unit->IsBelongsToFirst ());
        target = {nextWaypoint.x_, 0.0f, nextWaypoint.y_};
    }
    else
    {
        Unit *another = unitsManager->GetUnit (command.argument_);
        if (another == nullptr)
        {
            throw UniversalException <UnitsManager> ("UnitsManager: unit " + Urho3D::String (command.argument_) +
                                                     " does not exists, can not follow! AI error?");
        }
        target = another->GetNode ()->GetWorldPosition ();
    }

    target = unit->GetScene ()->GetComponent <Urho3D::NavigationMesh> ()->FindNearestPoint (target,
                                                                                            Urho3D::Vector3 (1.0f, INT_MAX, 1.0f));

    Urho3D::CrowdAgent *crowdAgent = unit->GetNode ()->GetComponent <Urho3D::CrowdAgent> ();
    crowdAgent->SetTargetPosition (target);
}

void ProcessUnitCommandAttackUnit (UnitsManager *unitsManager, Unit *unit, const UnitCommand &command,
                                   const UnitType &unitType)
{
    Urho3D::CrowdAgent *crowdAgent = unit->GetNode ()->GetComponent <Urho3D::CrowdAgent> ();
    crowdAgent->SetTargetVelocity (Urho3D::Vector3::ZERO);

    Unit *another = unitsManager->GetUnit (command.argument_);
    if (another == nullptr)
    {
        throw UniversalException <UnitsManager> ("UnitsManager: unit " + Urho3D::String (command.argument_) +
                                                 " does not exists, can not attack! AI error?");
    }

    const UnitType &anotherUnitType = unitsManager->GetUnitType (another->GetUnitType ());
    if ((unit->GetNode ()->GetWorldPosition () - another->GetNode ()->GetWorldPosition ()).Length () >
        anotherUnitType.GetNavigationRadius () + unitType.GetAttackRange () + unitType.GetNavigationRadius ())
    {
        throw UniversalException <UnitsManager> ("UnitsManager: unit " + Urho3D::String (command.argument_) +
                                                 " is too far, can not attack! AI error?");
    }

    if (unit->GetAttackCooldown () <= 0.0f)
    {
        float attackModifier = unitType.GetAttackModiferVersus (another->GetUnitType ());
        another->SetHp (static_cast <unsigned int> (another->GetHp () > unitType.GetAttackForce () * attackModifier ?
                        another->GetHp () - unitType.GetAttackForce () * attackModifier : 0));
        unit->SetAttackCooldown (unitType.GetAttackSpeed ());
    }
}
}
