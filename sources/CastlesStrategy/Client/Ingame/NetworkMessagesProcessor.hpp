#pragma once
#include <Urho3D/Core/Context.h>

namespace CastlesStrategy
{
class IngameActivity;
typedef void (*ClientIncomingNetworkMessageProcessor) (IngameActivity *ingameActivity, Urho3D::VectorBuffer &messageData);

class NetworkMessagesProcessor : public Urho3D::Object
{
URHO3D_OBJECT (NetworkMessagesProcessor, Object)
public:
    explicit NetworkMessagesProcessor (IngameActivity *owner);
    virtual ~NetworkMessagesProcessor ();

private:
    void HandleNetworkMessage (Urho3D::StringHash eventType, Urho3D::VariantMap &data);

    IngameActivity *owner_;
    Urho3D::PODVector <ClientIncomingNetworkMessageProcessor> incomingMessagesProcessors_;
};
}
