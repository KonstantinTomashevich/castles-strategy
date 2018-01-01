#pragma once
#include <vector>

#include <CastlesStrategyServer/Managers/Manager.hpp>
#include <CastlesStrategyServer/Map/Route.hpp>

namespace CastlesStrategy
{
class Map : public Manager
{
public:
    Map (ManagersHub *managersHub);
    virtual ~Map ();

    virtual void HandleUpdate (float timeStep);
    const Urho3D::IntVector2 &GetSize () const;
    void SetSize (const Urho3D::IntVector2 &size);

    unsigned int GetInvertedPlayer () const;
    void SetInvertedPlayer (unsigned int invertedPlayer);

    const std::vector <Route> &GetRoutes () const;
    Urho3D::Vector2 GetWaypoint (unsigned int route, unsigned int index, unsigned int ownerPlayer) const;

    void SaveRoutesToXML (Urho3D::XMLElement &output) const;
    void LoadRoutesFromXML (const Urho3D::XMLElement &input);

private:
    Urho3D::IntVector2 size_;
    unsigned int invertedPlayer_;
    std::vector <Route> routes_;
};
}
