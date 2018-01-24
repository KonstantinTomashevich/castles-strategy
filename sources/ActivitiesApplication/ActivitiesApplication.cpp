#include "ActivitiesApplication.hpp"
#include <Urho3D/Core/CoreEvents.h>
#include <Utils/UniversalException.hpp>

namespace ActivitiesApplication
{
ActivitiesApplication::ActivitiesApplication (Urho3D::Context *context) : Urho3D::Application (context),
    currentActivities_ (),
    activitiesToSetup_ (),
    activitiesToStop_ ()
{

}

ActivitiesApplication::~ActivitiesApplication ()
{

}

void ActivitiesApplication::Setup ()
{

}

void ActivitiesApplication::Start ()
{
    SubscribeToEvent (Urho3D::E_UPDATE, URHO3D_HANDLER (ActivitiesApplication, UpdateActivities));

    if (!currentActivities_.Empty ())
    {
        for (int index = 0; index < currentActivities_.Size (); index++)
        {
            currentActivities_.At (index)->Start ();
        }
    }
}

void ActivitiesApplication::UpdateActivities (Urho3D::StringHash eventType, Urho3D::VariantMap &eventData)
{
    float timeStep = eventData [Urho3D::Update::P_TIMESTEP].GetFloat ();
    if (!activitiesToStop_.Empty ())
    {
        for (Urho3D::SharedPtr <Activity> &activity : activitiesToStop_)
        {
            currentActivities_.Remove (activity);
            activity->Stop ();
        }
        activitiesToStop_.Clear ();
    }

    if (!activitiesToSetup_.Empty ())
    {
        for (Urho3D::SharedPtr <Activity> &activity : activitiesToSetup_)
        {
            currentActivities_.Push (activity);
            activity->SetApplication (this);
            activity->Start ();
        }
        activitiesToSetup_.Clear ();
    }

    if (!currentActivities_.Empty ())
    {
        for (int index = 0; index < currentActivities_.Size (); index++)
        {
            currentActivities_.At (index)->Update (timeStep);
        }
    }
}

void ActivitiesApplication::Stop ()
{
    if (!currentActivities_.Empty ())
    {
        for (int index = 0; index < currentActivities_.Size (); index++)
        {
            currentActivities_.At (index)->Stop ();
        }
    }
}

void ActivitiesApplication::SetupActivityNextFrame (Activity *activity)
{
    if (activity == nullptr)
    {
        throw UniversalException <ActivitiesApplication> ("ActivitiesApplication: can not setup nullptr activity!");
    }
    activitiesToSetup_.Push (Urho3D::SharedPtr <Activity> (activity));
}

void ActivitiesApplication::StopActivityNextFrame (Activity *activity)
{
    if (activity == nullptr)
    {
        throw UniversalException <ActivitiesApplication> ("ActivitiesApplication: can not stop nullptr activity!");
    }
    activitiesToStop_.Push (Urho3D::SharedPtr <Activity> (activity));
}

unsigned ActivitiesApplication::GetActivitiesCount ()
{
    return currentActivities_.Size ();
}

Activity *ActivitiesApplication::GetActivityByIndex (int index)
{
    if (index >= currentActivities_.Size ())
    {
        throw UniversalException <ActivitiesApplication> ("ActivitiesApplication: there is only" +
                                                 Urho3D::String (currentActivities_.Size ()) + " but activity with index " +
                                                 Urho3D::String (index) + "requested!"
        );
    }
    return currentActivities_.At (index);
}
}

