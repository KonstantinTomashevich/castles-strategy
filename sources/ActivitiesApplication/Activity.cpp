#include "Activity.hpp"
#include <ActivitiesApplication/ActivitiesApplication.hpp>

namespace ActivitiesApplication
{
Activity::Activity (Urho3D::Context *context) : Urho3D::Object (context), application_ (0)
{

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
    application_ = application;
}
}
