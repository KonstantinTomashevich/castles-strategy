#pragma once
#include <vector>
#include <CastlesStrategy/Server/Managers/Manager.hpp>
#include <CastlesStrategy/Shared/Village/Village.hpp>

namespace CastlesStrategy
{
const float DEFAULT_TAXES_DELAY = 1.0f;

class VillagesManager : public Manager
{
public:
    explicit VillagesManager (ManagersHub *managersHub);
    virtual ~VillagesManager ();
    virtual void HandleUpdate (float timeStep);

    float GetTimeUntilTaxes () const;
    unsigned int GetVillagesCount () const;
    const Village *GetVillageById (unsigned int id) const;

    Village *GetVillageById (unsigned int id);
    const Urho3D::PODVector <Village *> &GetVillages () const;
    Village *CreateVillage (const Urho3D::Vector3 &position);

    void SaveVillagesToXML (Urho3D::XMLElement &output) const;
    void LoadVillagesFromXML (const Urho3D::XMLElement &input);

private:
    unsigned GetVillageIndex (unsigned id, bool &found) const;
    void UpdateVillagesOwnerships (float timeStep) const;
    void ProcessTaxes ();

    float timeUntilTaxes_;
    Urho3D::PODVector <Village *> villages_;
};
}
