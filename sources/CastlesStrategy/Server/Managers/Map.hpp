#pragma once
#include <vector>

#include <CastlesStrategy/Server/Managers/Manager.hpp>
#include <CastlesStrategy/Server/Map/Route.hpp>

namespace CastlesStrategy
{
class Map : public Manager
{
public:
    explicit Map (ManagersHub *managersHub);
    virtual ~Map ();

    virtual void HandleUpdate (float timeStep);
    const Urho3D::IntVector2 &GetSize () const;
    void SetSize (const Urho3D::IntVector2 &size);

    const std::vector <Route> &GetRoutes () const;
    Urho3D::Vector2 GetWaypoint (unsigned int route, unsigned int index, bool isBelongsToFirst) const;

    void SaveRoutesToXML (Urho3D::XMLElement &output) const;
    void LoadRoutesFromXML (const Urho3D::XMLElement &input);

private:
    Urho3D::IntVector2 size_;
    std::vector <Route> routes_;
};
}
