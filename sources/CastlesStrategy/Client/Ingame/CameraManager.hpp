#pragma once
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/Node.h>

namespace CastlesStrategy
{
typedef int KeyCode;
typedef int MouseButtonCode;
class IngameActivity;

class CameraManager : public Urho3D::Object
{
URHO3D_OBJECT (CameraManager, Object)
public:
    CameraManager (IngameActivity *owner);
    virtual ~CameraManager ();

    void SetupCamera (const Urho3D::Vector3 &position, const Urho3D::Quaternion &rotation);
    void Update (float timeStep);

    float GetMoveSpeed () const;
    void SetMoveSpeed (float moveSpeed);

    MouseButtonCode GetMouseButtonMove () const;

    void SetMouseButtonMove (MouseButtonCode mouseButtonMove);

    KeyCode GetKeyForward () const;
    void SetKeyForward (KeyCode keyForward);

    KeyCode GetKeyBackward () const;
    void SetKeyBackward (KeyCode keyBackward);

    KeyCode GetKeyLeft () const;
    void SetKeyLeft (KeyCode keyLeft);

    KeyCode GetKeyRight () const;
    void SetKeyRight (KeyCode keyRight);

private:
    IngameActivity *owner_;
    Urho3D::Node *cameraNode_;

    float moveSpeed_;
    MouseButtonCode mouseButtonMove_;
    KeyCode keyForward_;
    KeyCode keyBackward_;
    KeyCode keyLeft_;
    KeyCode keyRight_;
};
}

