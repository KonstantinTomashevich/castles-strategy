#include "VillagesManager.hpp"
#include <Urho3D/Scene/Node.h>
#include <CastlesStrategy/Server/Managers/ManagersHub.hpp>
#include <CastlesStrategy/Server/Managers/PlayersManager.hpp>
#include <Utils/UniversalException.hpp>

namespace CastlesStrategy
{
VillagesManager::VillagesManager (ManagersHub *managersHub) : Manager (managersHub),
    timeUntilTaxes_ (DEFAULT_TAXES_DELAY),
    villages_ ()
{

}

VillagesManager::~VillagesManager ()
{

}

void VillagesManager::HandleUpdate (float timeStep)
{
    UpdateVillagesOwnerships (timeStep);
    if (timeUntilTaxes_ <= 0.0f)
    {
        ProcessTaxes ();
    }
}



float VillagesManager::GetTimeUntilTaxes () const
{
    return timeUntilTaxes_;
}

unsigned int VillagesManager::GetVillagesCount () const
{
    return villages_.Size ();
}

const Village *VillagesManager::GetVillageById (unsigned int id) const
{
    bool found;
    unsigned index = GetVillageIndex (id, found);
    return found ? villages_ [index] : nullptr;
}

Village *VillagesManager::GetVillageById (unsigned int id)
{
    bool found;
    unsigned index = GetVillageIndex (id, found);
    return found ? villages_ [index] : nullptr;
}

const Urho3D::PODVector <Village *> &VillagesManager::GetVillages () const
{
    return villages_;
}

Village *VillagesManager::CreateVillage (const Urho3D::Vector3 &position)
{
    Urho3D::Node *villagesNode = GetManagersHub ()->GetScene ()->GetChild ("villages");
    if (villagesNode == nullptr)
    {
        villagesNode = GetManagersHub ()->GetScene ()->CreateChild ("villages", Urho3D::REPLICATED);
    }

    Urho3D::Node *newVillageNode = villagesNode->CreateChild ("village", Urho3D::REPLICATED);
    newVillageNode->SetWorldPosition (position);

    Village *newVillage = newVillageNode->CreateComponent <Village> (Urho3D::REPLICATED);
    villages_.Push (newVillage);
    return newVillage;
}

void VillagesManager::SaveVillagesToXML (Urho3D::XMLElement &output) const
{
    for (Village *village : villages_)
    {
        Urho3D::XMLElement newChild = output.CreateChild ("village");
        village->SaveXML (newChild);
        newChild.SetVector3 ("position", village->GetNode ()->GetWorldPosition ());
    }
}

void VillagesManager::LoadVillagesFromXML (const Urho3D::XMLElement &input)
{
    Urho3D::XMLElement villageXML = input.GetChild ("village");
    villages_.Clear ();

    while (villageXML.NotNull ())
    {
        Village *newVillage = CreateVillage (villageXML.GetVector3 ("position"));
        newVillage->LoadXML (villageXML);
        villageXML = villageXML.GetNext ("village");
    }
}

unsigned VillagesManager::GetVillageIndex (unsigned id, bool &found) const
{
    unsigned int left = 0;
    unsigned int right = villages_.Size () - 1;

    while (left <= right)
    {
        unsigned int medium = left + (right - left) / 2;
        Village *village = villages_ [medium];

        if (village->GetID () == id)
        {
            found = true;
            return medium;
        }
        else if (village->GetID () > id)
        {
            right = medium - 1;
        }
        else
        {
            left = medium + 1;
        }
    }

    found = false;
    return 0;
}

void VillagesManager::UpdateVillagesOwnerships (float timeStep) const
{
    for (Village *village : villages_)
    {
        village->UpdateOwnership (timeStep,
                dynamic_cast <const UnitsManager *> (GetManagersHub ()->GetManager (MI_UNITS_MANAGER)));
    }
}

void VillagesManager::ProcessTaxes ()
{
    unsigned int firstPlayerCoins = 0;
    unsigned int secondPlayerCoins = 0;

    for (Village *& village : villages_)
    {
        if (village->GetOwnership () > 0.0f)
        {
            firstPlayerCoins += village->TakeCoins (DEFAULT_TAXES_DELAY);
        }
        else
        {
            secondPlayerCoins += village->TakeCoins (DEFAULT_TAXES_DELAY);
        }
    }

    PlayersManager *playersManager =
            dynamic_cast <PlayersManager *> (GetManagersHub ()->GetManager (MI_PLAYERS_MANAGER));
    Player &firstPlayer = playersManager->GetFirstPlayer ();
    Player &secondPlayer = playersManager->GetSecondPlayer ();

    firstPlayer.SetCoins (firstPlayer.GetCoins () + firstPlayerCoins);
    secondPlayer.SetCoins (secondPlayer.GetCoins () + firstPlayerCoins);
    timeUntilTaxes_ = DEFAULT_TAXES_DELAY;
}
}
