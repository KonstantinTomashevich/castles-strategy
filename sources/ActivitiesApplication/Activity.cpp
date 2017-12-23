#include "Activity.hpp"
#include <ActivitiesApplication/ActivitiesApplication.hpp>
#include <ActivitiesApplication/Exception.hpp>

namespace ActivitiesApplication
{
Activity::Activity (Urho3D::Context *context) : Urho3D::Object (context), application_ (0)
{
    if (context == nullptr)
    {
        throw Exception <Activity> ("Activity: context pointer can not be null!");
    }
}

Activity::~Activity ()
{

}

ActivitiesApplication *Activity::GetApplication()
{
    return application_;
}

void Activity::SetApplication (ActivitiesApplication *application)
{
    if (application == nullptr)
    {
        throw Exception <Activity> ("Activity: application pointer can not be null!");
    }
    application_ = application;
}
}
