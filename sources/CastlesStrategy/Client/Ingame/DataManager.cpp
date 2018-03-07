#include "DataManager.hpp"
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Resource/ResourceCache.h>

#include <Utils/UniversalException.hpp>
#include <CastlesStrategy/Client/Ingame/IngameActivity.hpp>

namespace CastlesStrategy
{
DataManager::DataManager (IngameActivity *owner) : Urho3D::Object (owner->GetContext ()),
        owner_ (owner),
        unitsTypes_ ()
{

}

DataManager::~DataManager ()
{
    UnsubscribeFromAllEvents ();
}

void DataManager::Update (float timeStep)
{
    AttemptToAddPrefabs ();
}

void DataManager::AddPrefabToUnit (unsigned int nodeID)
{
    unitNodesToAddPrefabs_.Insert (nodeID);
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
}
