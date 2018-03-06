#include "DataManager.hpp"
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
        throw UniversalException <DataManager> ("DataManager: can not find scene xml!");
    }

    return unitsTypes_ [index];
}
}
