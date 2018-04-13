#pragma once
#include <vector>
#include <Urho3D/Core/Context.h>
#include <CastlesStrategy/Shared/Unit/UnitType.hpp>
#include <CastlesStrategy/Shared/PlayerType.hpp>

namespace CastlesStrategy
{
class IngameActivity;
struct RecruitmentOrder
{
    unsigned int unitType_;
    float timeLeft_;
};

class DataManager : public Urho3D::Object
{
URHO3D_OBJECT (DataManager, Object)
public:
    struct PlayerData
    {
        PlayerType playerType_;
        bool readyForStart_;
    };

    explicit DataManager (IngameActivity *owner);
    virtual ~DataManager ();

    void Update (float timeStep);
    void AddPrefabToObject (unsigned int nodeID);
    void RecruitUnit (unsigned int unitType);
    void SpawnUnit (unsigned int unitType);
    void LoadMapResources ();

    const Urho3D::String &GetMapName () const;
    void SetMapName (const Urho3D::String &mapName);

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

    Urho3D::Node *GetSelectedSpawnNode () const;
    void SetSelectedSpawnNode (Urho3D::Node *selectedSpawnNode);
    unsigned int GetPredictedOrdedUnitsCount (unsigned int unitType) const;

    void AddPlayer (const Urho3D::String &name, PlayerType playerType, bool readyForStart);
    void RemovePlayer (const Urho3D::String &name);
    void SetPlayerType (const Urho3D::String &name, PlayerType playerType);
    void SetIsPlayerReadyForStart (const Urho3D::String &name, bool readyForStart);
    const Urho3D::HashMap <Urho3D::String, DataManager::PlayerData> &GetPlayers ();

private:
    void AttemptToAddPrefabs ();
    void PredictOrders (float timeStep);

    IngameActivity *owner_;
    Urho3D::String mapName_;
    unsigned int spawnsUnitType_;
    std::vector <UnitType> unitsTypes_;

    Urho3D::HashSet <unsigned int> objectsNodesToAddPrefabs_;
    Urho3D::PODVector <unsigned int> predictedUnitsPull_;
    unsigned int predictedCoins_;
    Urho3D::Node *selectedSpawnNode_;

    Urho3D::Vector <RecruitmentOrder> predictedOrders_;
    Urho3D::Vector <unsigned int> predictedOrderedUnitsCounts_;
    Urho3D::HashMap <Urho3D::String, PlayerData> players_;
};
}
