#pragma once
#include <vector>
#include <Urho3D/Core/Context.h>
#include <CastlesStrategy/Shared/Unit/UnitType.hpp>

namespace CastlesStrategy
{
class IngameActivity;
class DataManager : public Urho3D::Object
{
URHO3D_OBJECT (DataManager, Object)
public:
    explicit DataManager (IngameActivity *owner);
    virtual ~DataManager ();

    void Update (float timeStep);
    void AddPrefabToUnit (unsigned int nodeID);

    unsigned int GetSpawnsUnitType () const;
    void SetSpawnsUnitType (unsigned int spawnsUnitType);

    void LoadUnitsTypesFromXML (const Urho3D::XMLElement &input);
    const std::vector <UnitType> &GetUnitsTypes () const;

    unsigned int GetUnitsTypesCount () const;
    const UnitType &GetUnitTypeByIndex (unsigned int index) const;

    void UpdateUnitsPull (unsigned int unitType, unsigned int newValue);
    unsigned int GetPredictedUnitsInPull (unsigned int unitType) const;

    unsigned int GetPredictedCoins () const;
    void SetPredictedCoins (unsigned int predictedCoins);

private:
    void AttemptToAddPrefabs ();

    IngameActivity *owner_;
    unsigned int spawnsUnitType_;
    std::vector <UnitType> unitsTypes_;

    Urho3D::HashSet <unsigned int> unitNodesToAddPrefabs_;
    Urho3D::PODVector <unsigned int> predictedUnitsPull_;
    unsigned int predictedCoins_;
};
}
