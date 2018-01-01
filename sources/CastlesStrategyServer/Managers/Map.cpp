#include "Map.hpp"
#include <ActivitiesApplication/UniversalException.hpp>

namespace CastlesStrategy
{
Map::Map (ManagersHub *managersHub) : Manager (managersHub),
    invertedPlayer_ (0),
    routes_ ()
{

}

Map::~Map ()
{

}

void Map::HandleUpdate (float timeStep)
{

}

const Urho3D::IntVector2 &Map::GetSize () const
{
    return size_;
}

void Map::SetSize (const Urho3D::IntVector2 &size)
{
    size_ = size;
}

unsigned int Map::GetInvertedPlayer () const
{
    return invertedPlayer_;
}

void Map::SetInvertedPlayer (unsigned int invertedPlayer)
{
    invertedPlayer_ = invertedPlayer;
}

const std::vector <Route> &Map::GetRoutes () const
{
    return routes_;
}

Urho3D::Vector2 Map::GetWaypoint (unsigned int route, unsigned int index, unsigned int ownerPlayer) const
{
    if (route >= routes_.size ())
    {
        throw UniversalException <Map> ("Map: requested route " + Urho3D::String (route) + ", but there is only " +
            Urho3D::String (routes_.size ()) + " routes!");
    }

    const Route &requestedRoute = routes_ [route];
    if (index >= requestedRoute.GetWaypoints ().Size ())
    {
        throw UniversalException <Map> ("Map: requested route waypoint " + Urho3D::String (index) + ", but there is only " +
                                                Urho3D::String (requestedRoute.GetWaypoints ().Size ()) + " waypoints!");
    }

    return requestedRoute.GetWaypoints () [ownerPlayer != invertedPlayer_ ?
                                           index : requestedRoute.GetWaypoints ().Size () - index - 1];
}

void Map::SaveRoutesToXML (Urho3D::XMLElement &output) const
{
    for (const Route &route : routes_)
    {
        Urho3D::XMLElement newChild = output.CreateChild ("route");
        route.SaveToXML (newChild);
    }
}

void Map::LoadRoutesFromXML (const Urho3D::XMLElement &input)
{
    Urho3D::XMLElement element = input.GetChild ("route");
    while (element.NotNull ())
    {
        routes_.push_back (Route::LoadFromXML (element));
        element = element.GetNext ("route");
    }
}
}
