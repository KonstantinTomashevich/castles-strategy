#pragma once
#include <vector>
#include <Urho3D/Container/Vector.h>

#include <CastlesStrategy/Server/Managers/Manager.hpp>
#include <CastlesStrategy/Shared/Network/GameStatus.hpp>
#include <CastlesStrategy/Shared/Unit/Unit.hpp>
#include <CastlesStrategy/Shared/Unit/UnitType.hpp>

namespace CastlesStrategy
{
URHO3D_EVENT (E_GAME_ENDED, GameEnded)
{
    URHO3D_PARAM (FIRST_WON, FirstWon);
}

class UnitsManager : public Manager
{
public:
    explicit UnitsManager (ManagersHub *managersHub);
    virtual ~UnitsManager ();

    void AddUnit (Unit *unit);
    const Unit *GetSpawn (unsigned route, bool belongsToFirst) const;

    const Unit *SpawnUnit (unsigned route, bool belongsToFirst, unsigned unitType);
    const Unit *SpawnUnit (unsigned spawnId, unsigned unitType);

    const Unit *GetUnit (unsigned int id) const;
    Unit *GetUnit (unsigned int id);
    const Unit *GetNearestEnemy (Unit *unit) const;
    Urho3D::PODVector <const Unit *> GetUnitsNear (Urho3D::Vector2 position, float radius) const;

    virtual void HandleUpdate (float timeStep);
    unsigned int GetUnitsTypesCount () const;
    const UnitType &GetUnitType (unsigned int index) const;
    unsigned int GetSpawnsUnitType () const;

    void SaveUnitsTypesToXML (Urho3D::XMLElement &output) const;
    void LoadUnitsTypesFromXML (const Urho3D::XMLElement &input);

    void SaveSpawnsToXML (Urho3D::XMLElement &output) const;
    void LoadSpawnsFromXML (const Urho3D::XMLElement &input);

private:
    const Unit *SpawnUnit (const Unit *spawn, unsigned unitType);
    unsigned GetUnitIndex (unsigned id, bool &found) const;
    void ProcessUnits (float timeStep);
    void ClearDeadUnits ();

    Unit *CreateUnit (Urho3D::Vector2 position, unsigned unitType, bool belongsToFirst, unsigned route);
    void SetupUnit (Unit *unit);
    void ProcessUnitCommand (Unit *unit, const UnitCommand &command, const UnitType &unitType);
    void MakeUnitDead (Unit *unit);

    friend void ProcessUnitCommandMoveOrFollow (
            UnitsManager *unitsManager, Unit *unit, const UnitCommand &command, const UnitType &unitType);

    friend void ProcessUnitCommandAttackUnit (
            UnitsManager *unitsManager, Unit *unit, const UnitCommand &command, const UnitType &unitType);

    typedef void (*UnitCommandProcessor) (
            UnitsManager *unitsManager, Unit *unit, const UnitCommand &command, const UnitType &unitType);

    unsigned spawnsUnitType_;
    std::vector <UnitType> unitsTypes_;
    Urho3D::PODVector <Unit *> units_;
    Urho3D::PODVector <UnitCommandProcessor> unitCommandProcessors_;
};
}
