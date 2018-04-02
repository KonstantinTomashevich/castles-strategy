#pragma once
#include <Urho3D/Container/Vector.h>
#include <Urho3D/Network/Connection.h>
#include <CastlesStrategy/Server/Managers/ManagersHub.hpp>

namespace CastlesStrategy
{
class ServerActivity;
namespace IncomingNetworkMessageProcessors
{
void AddOrder (ServerActivity *activity, Urho3D::VectorBuffer &messageData, Urho3D::Connection *sender);
void SpawnUnit (ServerActivity *activity, Urho3D::VectorBuffer &messageData, Urho3D::Connection *sender);
void ChatMessage (ServerActivity *activity, Urho3D::VectorBuffer &messageData, Urho3D::Connection *sender);

void RequestToChangeType (ServerActivity *activity, Urho3D::VectorBuffer &messageData, Urho3D::Connection *sender);
void SetIsReadyForStart (ServerActivity *activity, Urho3D::VectorBuffer &messageData, Urho3D::Connection *sender);
}
}
