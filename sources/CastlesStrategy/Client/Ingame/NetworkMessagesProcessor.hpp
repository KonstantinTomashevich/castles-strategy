#pragma once
#include <Urho3D/Core/Context.h>

namespace CastlesStrategy
{
class IngameActivity;
class NetworkMessagesProcessor : public Urho3D::Object
{
URHO3D_OBJECT (NetworkMessagesProcessor, Object)
public:
    explicit NetworkMessagesProcessor (IngameActivity *owner);
    virtual ~NetworkMessagesProcessor ();

private:
    void HandleNetworkMessage (Urho3D::StringHash eventType, Urho3D::VariantMap &data);
    void ProcessGameStatusMessage (Urho3D::VectorBuffer &messageData);
    void ProcessUnitsTypesXMLMessage (Urho3D::VectorBuffer &messageData);

    IngameActivity *owner_;
};
}
