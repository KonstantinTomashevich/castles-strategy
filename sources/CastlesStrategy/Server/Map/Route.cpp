#include "Route.hpp"
#include <Utils/UniversalException.hpp>

namespace CastlesStrategy
{
Route::Route (const Urho3D::PODVector <Urho3D::Vector2> &waypoints) :
    waypoints_ (waypoints)
{
    if (waypoints_.Empty ())
    {
        throw UniversalException <Route> ("Route: route must contain at least one waypoint!");
    }
}

Route::Route (const Route &another) :
    waypoints_ (another.waypoints_)
{

}

Route::~Route ()
{

}

const Urho3D::PODVector <Urho3D::Vector2> &Route::GetWaypoints () const
{
    return waypoints_;
}

void Route::SaveToXML (Urho3D::XMLElement &output) const
{
    for (const Urho3D::Vector2 &waypoint : waypoints_)
    {
        Urho3D::XMLElement newChild = output.CreateChild ("waypoint");
        newChild.SetVector2 ("coord", waypoint);
    }
}

Route Route::LoadFromXML (const Urho3D::XMLElement &input)
{
    Urho3D::PODVector <Urho3D::Vector2> waypoints;
    Urho3D::XMLElement element = input.GetChild ("waypoint");
    while (element.NotNull ())
    {
        waypoints.Push (element.GetVector2 ("coord"));
        element = element.GetNext ("waypoint");
    }

    return Route (waypoints);
}
}
