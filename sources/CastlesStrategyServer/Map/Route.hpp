#pragma once
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Math/Vector2.h>
#include <Urho3D/Resource/XMLElement.h>

namespace CastlesStrategy
{
class Route
{
public:
    explicit Route (const Urho3D::PODVector <Urho3D::Vector2> &waypoints);
    virtual ~Route ();

    const Urho3D::PODVector <Urho3D::Vector2> &GetWaypoints () const;
    void SaveToXML (Urho3D::XMLElement &output) const;
    static Route LoadFromXML (const Urho3D::XMLElement &input);

private:
    Urho3D::PODVector <Urho3D::Vector2> waypoints_;
};
}
