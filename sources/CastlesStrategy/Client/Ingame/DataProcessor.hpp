#pragma once
#include <vector>
#include <Urho3D/Core/Context.h>
#include <CastlesStrategy/Shared/Unit/UnitType.hpp>

namespace CastlesStrategy
{
class IngameActivity;
class DataProcessor : public Urho3D::Object
{
URHO3D_OBJECT (DataProcessor, Object)
public:
    explicit DataProcessor (IngameActivity *owner);
    virtual ~DataProcessor ();

    unsigned int GetSpawnsUnitType () const;
    void SetSpawnsUnitType (unsigned int spawnsUnitType);

    void LoadUnitsTypesFromXML (const Urho3D::XMLElement &input);
    const std::vector <UnitType> &GetUnitsTypes () const;

    unsigned int GetUnitsTypesCount () const;
    const UnitType &GetUnitTypeByIndex (unsigned int index) const;

private:
    IngameActivity *owner_;
    unsigned int spawnsUnitType_;
    std::vector <UnitType> unitsTypes_;
};
}
