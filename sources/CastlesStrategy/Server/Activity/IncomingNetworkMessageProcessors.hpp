#pragma once
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Network/Connection.h>
#include <CastlesStrategy/Server/Managers/ManagersHub.hpp>

namespace CastlesStrategy
{
namespace IncomingNetworkMessageProcessors
{
void AddOrder (ManagersHub *managersHub,
               const Urho3D::Vector <Urho3D::Pair <Urho3D::Connection *, Urho3D::String> > &identifiedConnections);

void SpawnUnit (ManagersHub *managersHub,
                const Urho3D::Vector <Urho3D::Pair <Urho3D::Connection *, Urho3D::String> > &identifiedConnections);
}
}
