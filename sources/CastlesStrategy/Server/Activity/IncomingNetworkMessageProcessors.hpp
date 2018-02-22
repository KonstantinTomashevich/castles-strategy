#pragma once
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Network/Connection.h>
#include <CastlesStrategy/Server/Managers/ManagersHub.hpp>

namespace CastlesStrategy
{
namespace IncomingNetworkMessageProcessors
{
void AddOrder (ManagersHub *managersHub,
        const Urho3D::HashMap <Urho3D::Connection *, Urho3D::String> &identifiedConnections, Urho3D::Connection *sender);

void SpawnUnit (ManagersHub *managersHub,
        const Urho3D::HashMap <Urho3D::Connection *, Urho3D::String> &identifiedConnections, Urho3D::Connection *sender);
}
}
