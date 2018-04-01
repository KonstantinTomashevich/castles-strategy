#include "DataManager.hpp"
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Resource/ResourceCache.h>

#include <Utils/UniversalException.hpp>
#include <CastlesStrategy/Client/Ingame/IngameActivity.hpp>

namespace CastlesStrategy
{
DataManager::DataManager (IngameActivity *owner) : Urho3D::Object (owner->GetContext ()),
        owner_ (owner),
        unitsTypes_ (),
        spawnsUnitType_ (0),

        unitNodesToAddPrefabs_ (),
        predictedUnitsPull_ (),
        predictedCoins_ (0),
        selectedSpawnNode_ (nullptr),

        predictedOrders_ (),
        predictedOrderedUnitsCounts_ (),
        players_ ()
{

}

DataManager::~DataManager ()
{
    UnsubscribeFromAllEvents ();
}

void DataManager::Update (float timeStep)
{
    AttemptToAddPrefabs ();
    PredictOrders (timeStep);
}

void DataManager::AddPrefabToUnit (unsigned int nodeID)
{
    unitNodesToAddPrefabs_.Insert (nodeID);
}

void DataManager::RecruitUnit (unsigned int unitType)
{
    const UnitType &typeInfo = GetUnitTypeByIndex (unitType);
    if (predictedCoins_ < typeInfo.GetRecruitmentCost ())
    {
        throw UniversalException <DataManager> ("DataManager: can not recruit unit type " + Urho3D::String (unitType) +
                ", required " + Urho3D::String (typeInfo.GetRecruitmentCost ()) + " coins, "
                "but only " + Urho3D::String (predictedCoins_) + " coins available!"
        );
    }

    predictedOrders_.Push ({unitType, typeInfo.GetRecruitmentTime ()});
    predictedOrderedUnitsCounts_ [unitType]++;

    SetPredictedCoins (predictedCoins_ - typeInfo.GetRecruitmentCost ());
    owner_->GetNetworkManager ()->SendAddOrderMessage (unitType);
}

void DataManager::SpawnUnit (unsigned int unitType)
{
    if (selectedSpawnNode_ == nullptr)
    {
        throw UniversalException <DataManager> ("DataManager: can not spawn unit because selected spawn is nullptr!");
    }

    if (GetPredictedUnitsInPull (unitType) == 0)
    {
        throw UniversalException <DataManager> ("DataManager: can not spawn unit with type" +
                Urho3D::String (unitType) + ", because there is no units of this type in predicted pool!");
    }

    predictedUnitsPull_ [unitType]--;
    owner_->GetIngameUIManager ()->CheckUIForUnitsType (unitType);
    owner_->GetNetworkManager ()->SendSpawnMessage (selectedSpawnNode_->GetComponent <Unit> ()->GetID (), unitType);
}

unsigned int DataManager::GetSpawnsUnitType () const
{
    return spawnsUnitType_;
}

void DataManager::SetSpawnsUnitType (unsigned int spawnsUnitType)
{
    spawnsUnitType_ = spawnsUnitType;
}

void DataManager::LoadUnitsTypesFromXML (const Urho3D::XMLElement &input)
{
    unitsTypes_.clear ();
    spawnsUnitType_ = input.GetUInt ("spawnsUnitType");
    Urho3D::XMLElement element = input.GetChild ("unitType");
    unsigned int id = 0;

    while (element.NotNull ())
    {
        unitsTypes_.push_back (UnitType::LoadFromXML (id, element));
        element = element.GetNext ("unitType");
        id++;
    }

    predictedUnitsPull_.Resize (unitsTypes_.size ());
    predictedOrderedUnitsCounts_.Resize (unitsTypes_.size ());

    for (unsigned int index = 0; index < unitsTypes_.size (); index++)
    {
        predictedUnitsPull_ [index] = 0;
        predictedOrderedUnitsCounts_ [index] = 0;
    }
}

const std::vector <UnitType> &DataManager::GetUnitsTypes () const
{
    return unitsTypes_;
}

unsigned int DataManager::GetUnitsTypesCount () const
{
    return unitsTypes_.size ();
}

const UnitType &DataManager::GetUnitTypeByIndex (unsigned int index) const
{
    if (index >= unitsTypes_.size ())
    {
        throw UniversalException <DataManager> ("DataManager: requested unit type " + Urho3D::String (index) +
                " but there is only " + Urho3D::String (unitsTypes_.size ()) + " units types!");
    }

    return unitsTypes_ [index];
}

void DataManager::UpdateUnitsPull (unsigned int unitType, unsigned int newValue)
{
    if (unitType >= unitsTypes_.size ())
    {
        throw UniversalException <DataManager> ("DataManager: requested to update units pull of type " +
                Urho3D::String (unitType) + " but there is only " + Urho3D::String (unitsTypes_.size ()) +
                " units types!");
    }

    predictedUnitsPull_ [unitType] = newValue;
    if (unitType != spawnsUnitType_)
    {
        owner_->GetIngameUIManager ()->CheckUIForUnitsType (unitType);
    }
}

unsigned int DataManager::GetPredictedUnitsInPull (unsigned int unitType) const
{
    if (unitType >= unitsTypes_.size ())
    {
        throw UniversalException <DataManager> ("DataManager: requested units pull of type " +
                Urho3D::String (unitType) + " but there is only " + Urho3D::String (unitsTypes_.size ()) +
                " units types!");
    }

    return predictedUnitsPull_ [unitType];
}

unsigned int DataManager::GetPredictedCoins () const
{
    return predictedCoins_;
}

void DataManager::SetPredictedCoins (unsigned int predictedCoins)
{
    predictedCoins_ = predictedCoins;
    owner_->GetIngameUIManager ()->UpdateCoins (predictedCoins);
}

Urho3D::Node *DataManager::GetSelectedSpawnNode () const
{
    return selectedSpawnNode_;
}

void DataManager::SetSelectedSpawnNode (Urho3D::Node *selectedSpawnNode)
{
    if (selectedSpawnNode_ != nullptr)
    {
        selectedSpawnNode_->SetVar ("IsSelected", false);
    }

    selectedSpawnNode_ = selectedSpawnNode;
    if (selectedSpawnNode != nullptr)
    {
        PlayerType playerType = owner_->GetPlayerType ();
        Unit *unit = selectedSpawnNode->GetComponent <Unit> ();

        if (unit == nullptr || unit->GetUnitType () != spawnsUnitType_ || playerType == PT_OBSERVER ||
                (playerType == PT_FIRST && !unit->IsBelongsToFirst ()) ||
                (playerType == PT_SECOND && unit->IsBelongsToFirst ()))
        {
            selectedSpawnNode_ = nullptr;
        }
    }

    if (selectedSpawnNode_ != nullptr)
    {
        selectedSpawnNode_->SetVar ("IsSelected", true);
    }
    owner_->GetIngameUIManager ()->UpdateCoins (predictedCoins_);
}

unsigned int DataManager::GetPredictedOrdedUnitsCount (unsigned int unitType) const
{
    if (unitType >= unitsTypes_.size ())
    {
        throw UniversalException <DataManager> ("DataManager: requested ordered units count of type " +
                Urho3D::String (unitType) + " but there is only " + Urho3D::String (unitsTypes_.size ()) +
                " units types!");
    }

    return predictedOrderedUnitsCounts_ [unitType];
}

void DataManager::AddPlayer (const Urho3D::String &name, PlayerType playerType, bool readyForStart)
{
    auto iterator = players_.Find (name);
    if (iterator == players_.End ())
    {
        players_ [name] = {playerType, readyForStart};
        owner_->GetIngameUIManager ()->UpdatePlayersList ();
    }
    else
    {
        throw UniversalException <DataManager> ("DataManager: attempt to add " +
                name + ", but player with this name is already exists!");
    }
}

void DataManager::RemovePlayer (const Urho3D::String &name)
{
    auto iterator = players_.Find (name);
    if (iterator != players_.End ())
    {
        players_.Erase (iterator);
        owner_->GetIngameUIManager ()->UpdatePlayersList ();
    }
    else
    {
        throw UniversalException <DataManager> ("DataManager: attempt to remove " +
                name + ", but couldn't find player with this name!");
    }
}

void DataManager::SetPlayerType (const Urho3D::String &name, PlayerType playerType)
{
    auto iterator = players_.Find (name);
    if (iterator != players_.End ())
    {
        iterator->second_.playerType_ = playerType;
        owner_->GetIngameUIManager ()->UpdatePlayersList ();
    }
    else
    {
        throw UniversalException <DataManager> ("DataManager: attempt to set player type of " +
            name + ", but couldn't find player with this name!");
    }
}

void DataManager::SetIsPlayerReadyForStart (const Urho3D::String &name, bool readyForStart)
{
    auto iterator = players_.Find (name);
    if (iterator != players_.End ())
    {
        iterator->second_.readyForStart_ = readyForStart;
        owner_->GetIngameUIManager ()->UpdatePlayersList ();
    }
    else
    {
        throw UniversalException <DataManager> ("DataManager: attempt to set ready for start of " +
                name + ", but couldn't find player with this name!");
    }
}

const Urho3D::HashMap <Urho3D::String, DataManager::PlayerData> &DataManager::GetPlayers ()
{
    return players_;
}

void DataManager::AttemptToAddPrefabs ()
{
    auto iterator = unitNodesToAddPrefabs_.Begin ();
    while (iterator != unitNodesToAddPrefabs_.End ())
    {
        Urho3D::Node *node = owner_->GetScene ()->GetNode (*iterator);
        if (node != nullptr)
        {
            Unit *unit = node->GetComponent <Unit> ();
            if (unit != nullptr)
            {
                Urho3D::Node *prefab = node->CreateChild ("Prefab", Urho3D::LOCAL);
                const UnitType &unitType = GetUnitTypeByIndex (unit->GetUnitType ());
                Urho3D::XMLFile *prefabFile = context_->GetSubsystem <Urho3D::ResourceCache> ()->
                        GetResource <Urho3D::XMLFile> (unitType.GetPrefabPath ());

                if (prefabFile == nullptr || prefabFile->GetRoot ().IsNull ())
                {
                    throw UniversalException <DataManager> ("DataManager: requested prefab \"" +
                            unitType.GetPrefabPath () + "\" is not exists or is empty!");
                }

                prefab->LoadXML (prefabFile->GetRoot ());
                iterator = unitNodesToAddPrefabs_.Erase (iterator);
                continue;
            }
        }
        iterator++;
    }
}

void DataManager::PredictOrders (float timeStep)
{
    if (!predictedOrders_.Empty ())
    {
        RecruitmentOrder &order = predictedOrders_.Front ();
        order.timeLeft_ -= timeStep;

        if (order.timeLeft_ <= 0.0f)
        {
            predictedOrderedUnitsCounts_[order.unitType_]--;
            predictedOrders_.Erase (0);
        }
    }
}
}
