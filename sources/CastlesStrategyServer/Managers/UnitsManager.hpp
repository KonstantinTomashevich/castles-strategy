#pragma once
#include <vector>
#include <Urho3D/Container/Vector.h>

#include <CastlesStrategyServer/Managers/Manager.hpp>
#include <CastlesStrategyServer/Managers/GameStatus.hpp>
#include <CastlesStrategyServer/Unit/Unit.hpp>
#include <CastlesStrategyServer/Unit/UnitType.hpp>

namespace CastlesStrategy
{
class UnitsManager : public Manager
{
public:
    explicit UnitsManager (ManagersHub *managersHub);
    virtual ~UnitsManager ();

    void AddUnit (Unit *unit);
    unsigned SpawnUnit (unsigned spawnId, unsigned unitType);

    const Unit *GetUnit (unsigned int id) const;
    const Unit *GetNearestEnemy (Unit *unit) const;

    virtual void HandleUpdate (float timeStep);
    GameStatus CheckGameStatus () const;

    unsigned int GetUnitsTypesCount () const;
    const UnitType &GetUnitType (unsigned int index) const;

    void SaveUnitsTypesToXML (Urho3D::XMLElement &output) const;
    void LoadUnitsTypesFromXML (const Urho3D::XMLElement &input);

    void SaveSpawnsToXML (Urho3D::XMLElement &output) const;
    void LoadSpawnsFromXML (const Urho3D::XMLElement &input);

private:
    unsigned GetUnitIndex (unsigned id, bool &found) const;
    void ProcessUnits (float timeStep);
    void ClearDeadUnits ();

    Unit *CreateUnit (Urho3D::Vector2 position, unsigned unitType, bool belongsToFirst, unsigned route);
    void SetupUnit (Unit *unit);
    void ProcessUnitCommand (Unit *unit, const UnitCommand &command, const UnitType &unitType);
    void MakeUnitDead (Unit *unit);

    unsigned spawnUnitType_;
    std::vector <UnitType> unitsTypes_;
    Urho3D::PODVector <Unit *> units_;
};
}
