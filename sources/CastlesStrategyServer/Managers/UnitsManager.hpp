#pragma once
#include <vector>
#include <Urho3D/Container/Vector.h>

#include <CastlesStrategyServer/Managers/Manager.hpp>
#include <CastlesStrategyServer/Unit/Unit.hpp>
#include <CastlesStrategyServer/Unit/UnitType.hpp>

namespace CastlesStrategy
{
class UnitsManager : public Manager
{
public:
    UnitsManager (ManagersHub *managersHub);
    virtual ~UnitsManager ();

    void AddUnit (Unit *unit);
    const Unit *GetUnit (unsigned int id) const;
    const Unit *GetNearestEnemy (Unit *unit) const;
    virtual void HandleUpdate (float timeStep);

    void SaveUnitsTypesToXML (Urho3D::XMLElement &output) const;
    void LoadUnitsTypesFromXML (const Urho3D::XMLElement &input);

private:
    unsigned GetUnitIndex (unsigned id, bool &found) const;
    void ProcessUnits (float timeStep);
    void ClearDeadUnits ();

    void SetupUnit (Unit *unit);
    void ProcessUnitCommand (Unit *unit, const UnitCommand &command, const UnitType &unitType);
    void MakeUnitDead (Unit *unit);

    std::vector <UnitType> unitsTypes_;
    Urho3D::PODVector <Unit *> units_;
};
}
