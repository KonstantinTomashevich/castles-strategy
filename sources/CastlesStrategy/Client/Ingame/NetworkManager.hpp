#pragma once
#include <Urho3D/Core/Context.h>

namespace CastlesStrategy
{
class IngameActivity;
typedef void (*ClientIncomingNetworkMessageProcessor) (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);

class NetworkManager : public Urho3D::Object
{
URHO3D_OBJECT (NetworkManager, Object)
public:
    explicit NetworkManager (IngameActivity *owner);
    virtual ~NetworkManager ();

private:
    void HandleNetworkMessage (Urho3D::StringHash eventType, Urho3D::VariantMap &data);

    IngameActivity *owner_;
    Urho3D::PODVector <ClientIncomingNetworkMessageProcessor> incomingMessagesProcessors_;
};
}
