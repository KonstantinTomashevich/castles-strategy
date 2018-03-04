#include "CameraHandler.hpp"
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/IO/Log.h>

#include <Urho3D/Input/Input.h>
#include <CastlesStrategy/Client/Ingame/IngameActivity.hpp>

namespace CastlesStrategy
{
CameraHandler::CameraHandler (IngameActivity *owner) : Urho3D::Object (owner->GetContext ()),
    owner_ (owner),
    cameraNode_ (nullptr),
    moveSpeed_ (20.0f),
    mouseButtonMove_ (Urho3D::MOUSEB_MIDDLE),

    keyForward_ (Urho3D::KEY_W),
    keyBackward_ (Urho3D::KEY_S),
    keyLeft_ (Urho3D::KEY_A),
    keyRight_ (Urho3D::KEY_D)
{

}

CameraHandler::~CameraHandler ()
{

}

void CameraHandler::SetupCamera (const Urho3D::Vector3 &position, const Urho3D::Quaternion &rotation)
{
    cameraNode_ = owner_->GetScene ()->CreateChild ("Camera", Urho3D::LOCAL, 0);
    cameraNode_->SetPosition (position);
    cameraNode_->SetRotation (rotation);

    Urho3D::Camera *camera = cameraNode_->CreateComponent <Urho3D::Camera> (Urho3D::LOCAL);
    Urho3D::Renderer *renderer = context_->GetSubsystem <Urho3D::Renderer> ();
    renderer->SetNumViewports (1);
    renderer->SetViewport (0, new Urho3D::Viewport (context_, owner_->GetScene (), camera));
}

void CameraHandler::Update (float timeStep)
{
    if (cameraNode_ == nullptr)
    {
        return;
    }

    Urho3D::Input *input = context_->GetSubsystem <Urho3D::Input> ();
    Urho3D::Graphics *graphics = context_->GetSubsystem <Urho3D::Graphics> ();
    float deltaX = 0;
    float deltaZ = 0;

    if (input->GetMouseButtonDown (mouseButtonMove_))
    {
        deltaX = input->GetMousePosition ().x_ * 2.0f / graphics->GetWidth () - 1.0f;
        deltaZ = -(input->GetMousePosition ().y_ * 2.0f / graphics->GetHeight () - 1.0f);
    }
    else
    {
        if (input->GetKeyDown (keyForward_)) { deltaZ += 1.0f; }
        if (input->GetKeyDown (keyBackward_)) { deltaZ -= 1.0f; }

        if (input->GetKeyDown (keyRight_)) { deltaX += 1.0f; }
        if (input->GetKeyDown (keyLeft_)) { deltaX -= 1.0f; }
    }

    cameraNode_->Translate (
            Urho3D::Vector3 (moveSpeed_ * deltaX * timeStep, 0.0f, moveSpeed_ * deltaZ * timeStep), Urho3D::TS_WORLD);
}

float CameraHandler::GetMoveSpeed () const
{
    return moveSpeed_;
}

void CameraHandler::SetMoveSpeed (float moveSpeed)
{
    moveSpeed_ = moveSpeed;
}

MouseButtonCode CameraHandler::GetMouseButtonMove () const
{
    return mouseButtonMove_;
}

void CameraHandler::SetMouseButtonMove (MouseButtonCode mouseButtonMove)
{
    mouseButtonMove_ = mouseButtonMove;
}

KeyCode CameraHandler::GetKeyForward () const
{
    return keyForward_;
}

void CameraHandler::SetKeyForward (KeyCode keyForward)
{
    keyForward_ = keyForward;
}

KeyCode CameraHandler::GetKeyBackward () const
{
    return keyBackward_;
}

void CameraHandler::SetKeyBackward (KeyCode keyBackward)
{
    keyBackward_ = keyBackward;
}

KeyCode CameraHandler::GetKeyLeft () const
{
    return keyLeft_;
}

void CameraHandler::SetKeyLeft (KeyCode keyLeft)
{
    keyLeft_ = keyLeft;
}

KeyCode CameraHandler::GetKeyRight () const
{
    return keyRight_;
}

void CameraHandler::SetKeyRight (KeyCode keyRight)
{
    keyRight_ = keyRight;
}
}
