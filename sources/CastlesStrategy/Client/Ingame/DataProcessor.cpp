#include "DataProcessor.hpp"
#include <Utils/UniversalException.hpp>
#include <CastlesStrategy/Client/Ingame/IngameActivity.hpp>

namespace CastlesStrategy
{
DataProcessor::DataProcessor (IngameActivity *owner) : Urho3D::Object (owner->GetContext ()),
        unitsTypes_ ()
{

}

DataProcessor::~DataProcessor ()
{

}

unsigned int DataProcessor::GetSpawnsUnitType () const
{
    return spawnsUnitType_;
}

void DataProcessor::SetSpawnsUnitType (unsigned int spawnsUnitType)
{
    spawnsUnitType_ = spawnsUnitType;
}

void DataProcessor::LoadUnitsTypesFromXML (const Urho3D::XMLElement &input)
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

const std::vector <UnitType> &DataProcessor::GetUnitsTypes () const
{
    return unitsTypes_;
}

unsigned int DataProcessor::GetUnitsTypesCount () const
{
    return unitsTypes_.size ();
}

const UnitType &DataProcessor::GetUnitTypeByIndex (unsigned int index) const
{
    if (index >= unitsTypes_.size ())
    {
        throw UniversalException <DataProcessor> ("DataProcessor: can not find scene xml!");
    }

    return unitsTypes_ [index];
}
}
