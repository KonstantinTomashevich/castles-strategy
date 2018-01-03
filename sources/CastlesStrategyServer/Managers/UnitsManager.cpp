#include "UnitsManager.hpp"
#include <climits>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>

#include <Urho3D/Navigation/CrowdAgent.h>
#include <Urho3D/Navigation/NavigationMesh.h>
#include <Urho3D/IO/Log.h>

#include <CastlesStrategyServer/Unit/BasicUnitAI.hpp>
#include <CastlesStrategyServer/Managers/Map.hpp>
#include <CastlesStrategyServer/Managers/ManagersHub.hpp>
#include <ActivitiesApplication/UniversalException.hpp>

namespace CastlesStrategy
{
UnitsManager::UnitsManager (ManagersHub *managersHub) : Manager (managersHub),
    units_ (),
    unitsTypes_ ()
{

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

const Unit *UnitsManager::GetUnit (unsigned int id) const
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
        if (unit->GetOwner () != scanningUnit->GetOwner ())
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

void UnitsManager::SaveUnitsTypesToXML (Urho3D::XMLElement &output) const
{
    for (const UnitType &unitType : unitsTypes_)
    {
        Urho3D::XMLElement newChild = output.CreateChild ("unitType");
        unitType.SaveToXML (newChild);
    }
}

void UnitsManager::LoadUnitsTypesFromXML (const Urho3D::XMLElement &input)
{
    Urho3D::XMLElement element = input.GetChild ();
    unsigned int id = 0;

    while (element.NotNull ())
    {
        // TODO: Currently basic ai setted as default for all units types.
        unitsTypes_.push_back (UnitType::LoadFromXML (id, element));
        unitsTypes_.back ().SetAiProcessor (BasicUnitAI);

        element = element.GetNext ();
        id++;
    }
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

            const UnitType &unitType = unitsTypes_[unit->GetUnitType ()];
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

void UnitsManager::SetupUnit (Unit *unit)
{
    if (unit->GetUnitType () >= unitsTypes_.size ())
    {
        throw UniversalException <UnitsManager> ("UnitsManager: there is only " + Urho3D::String (unitsTypes_.size ()) +
                                                 " units types, but T" + Urho3D::String (unit->GetUnitType ()) + " requested!");
    }

    const UnitType &unitType = unitsTypes_ [unit->GetUnitType ()];
    unit->GetNode ()->SetVar (UNIT_PREFAB_VAR_HASH, unitType.GetPrefabPath ());
    Urho3D::CrowdAgent *crowdAgent = unit->GetNode ()->CreateComponent <Urho3D::CrowdAgent> (Urho3D::LOCAL);

    crowdAgent->SetMaxSpeed (unitType.GetMoveSpeed ());
    crowdAgent->SetMaxAccel (unitType.GetMoveSpeed ());
    crowdAgent->SetRadius (unitType.GetNavigationRadius ());

    unit->SetHp (unitType.GetMaxHp ());
    unit->SetAttackCooldown (0.0f);
    unit->SetCurrentWaypointIndex (0);
}

void UnitsManager::ProcessUnitCommand (Unit *unit, const UnitCommand &command, const UnitType &unitType)
{
    if (command.commandType_ == UCT_ATTACK_UNIT)
    {
        unit->GetNode ()->GetComponent <Urho3D::CrowdAgent> ()->SetTargetVelocity (Urho3D::Vector3::ZERO);
        Unit *another = nullptr;
        {
            bool found;
            unsigned index = GetUnitIndex (command.argument_, found);

            if (!found)
            {
                throw UniversalException <UnitsManager> ("UnitsManager: unit " + Urho3D::String (command.argument_) +
                                                         " does not exists, can not attack! AI error?");
            }
            else
            {
                another = units_[index];
            }
        }

        if ((unit->GetNode ()->GetWorldPosition () - another->GetNode ()->GetWorldPosition ()).Length () >
            unitType.GetAttackRange ())
        {
            throw UniversalException <UnitsManager> ("UnitsManager: unit " + Urho3D::String (command.argument_) +
                                                     " is too far, can not attack! AI error?");
        }

        if (unit->GetAttackCooldown () <= 0.0f)
        {
            another->SetHp (another->GetHp () > unitType.GetAttackForce () ?
                another->GetHp () - unitType.GetAttackForce () : 0);
            unit->SetAttackCooldown (unitType.GetAttackSpeed ());
        }
    }

    else if (command.commandType_ == UCT_MOVE_TO_WAYPOINT || command.commandType_ == UCT_FOLLOW_UNIT)
    {
        Urho3D::Vector3 target;
        if (command.commandType_ == UCT_MOVE_TO_WAYPOINT)
        {
            const Map *map = dynamic_cast <const Map *> (GetManagersHub ()->GetManager (MI_MAP));
            Urho3D::Vector2 nextWaypoint = map->GetWaypoint (
                    unit->GetRouteIndex (), unit->GetCurrentWaypointIndex (), unit->GetOwner ());
            target = {nextWaypoint.x_, 0.0f, nextWaypoint.y_};
        }
        else
        {
            Unit *another = nullptr;
            {
                bool found;
                unsigned index = GetUnitIndex (command.argument_, found);

                if (!found)
                {
                    throw UniversalException <UnitsManager> ("UnitsManager: unit " + Urho3D::String (command.argument_) +
                                                             " does not exists, can not follow! AI error?");
                }
                else
                {
                    another = units_[index];
                }
            }
            target = another->GetNode ()->GetWorldPosition ();
        }

        target = unit->GetScene ()->GetComponent <Urho3D::NavigationMesh> ()->FindNearestPoint (target,
            Urho3D::Vector3 (1.0f, INT_MAX, 1.0f));

        Urho3D::CrowdAgent *crowdAgent = unit->GetNode ()->GetComponent <Urho3D::CrowdAgent> ();
        crowdAgent->SetTargetPosition (target);
    }
}

void UnitsManager::MakeUnitDead (Unit *unit)
{
    // TODO: Implement.
}
}
