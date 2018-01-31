#pragma once
#include <Urho3D/Math/StringHash.h>

namespace CastlesStrategy
{
// TODO: Maybe allow to specify this vars in server activity (except P_IDENTITY_NAME of course).
const float AUTO_DISCONNECT_TIME = 1.0f;
const unsigned int SERVER_PORT = 10001;

namespace IdentityFields
{
const Urho3D::StringHash NAME ("Name");
}

const char *DEFAULT_MAPS_FOLDER = "Maps";
const char *DEFAULT_UNITS_TYPES_PATH = "DefaultUnitsTypes.xml";
}
