#pragma once
#include <Urho3D/Math/StringHash.h>

namespace CastlesStrategy
{
const float DEFAULT_AUTO_DISCONNECT_TIME = 1.0f;
const unsigned int DEFAULT_SERVER_PORT = 10001;

namespace IdentityFields
{
const Urho3D::StringHash NAME ("Name");
}

const Urho3D::String DEFAULT_MAPS_FOLDER ("Maps");
const Urho3D::String DEFAULT_UNITS_TYPES_PATH ("DefaultUnits/Types.xml");
}
