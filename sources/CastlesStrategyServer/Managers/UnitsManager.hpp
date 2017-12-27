#pragma once
#include <Urho3D/Container/Vector.h>
#include <CastlesStrategyServer/Unit/Unit.hpp>
#include <CastlesStrategyServer/Unit/UnitType.hpp>

namespace CastlesStrategy
{
class UnitsManager
{
public:
    UnitsManager ();
    virtual ~UnitsManager ();

    void AddUnit (Unit *unit);
    Unit *GetUnit (unsigned int id) const;
    void HandleUpdate (float timeStep);

    void SaveUnitsTypesToXML (Urho3D::XMLElement &output) const;
    void LoadUnitsTypesFromXML (const Urho3D::XMLElement &input);

private:
    unsigned GetUnitIndex (unsigned id, bool &found) const;
    void ProcessUnits (float timeStep);
    void ClearDeadUnits ();

    void SetupUnit (Unit *unit);
    void ProcessUnitCommand (Unit *unit, const UnitCommand &command, const UnitType &unitType);
    void MakeUnitDead (Unit *unit);

    Urho3D::PODVector <UnitType> unitsTypes_;
    Urho3D::PODVector <Unit *> units_;
};
}
