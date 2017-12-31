#include <Urho3D/Core/Context.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/IO/Log.h>

#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Model.h>

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Navigation/NavigationMesh.h>
#include <Urho3D/Navigation/CrowdManager.h>
#include <Urho3D/Navigation/Navigable.h>

#include <ActivitiesApplication/UniversalException.hpp>
#include <CastlesStrategyServer/Unit/Unit.hpp>
#include <CastlesStrategyServer/Unit/UnitType.hpp>
#include <CastlesStrategyServer/Managers/UnitsManager.hpp>

void CustomTerminate ();
void SetupEngine (Urho3D::Engine *engine);
Urho3D::Scene *SetupScene (Urho3D::Context *context);
void SetupUnitsManager (CastlesStrategy::UnitsManager &unitsManager, Urho3D::Context *context);
CastlesStrategy::Unit *SpawnUnit (Urho3D::Context *context, Urho3D::Scene *scene);

const Urho3D::PODVector <Urho3D::Vector2> waypoints = {{10.0f, 10.0f}, {-10.0f, 20.0f}, {10.0f, 30.0f}, {-10.0f, 40.0f}};
int main(int argc, char **argv)
{
    std::set_terminate (CustomTerminate);
    Urho3D::SharedPtr <Urho3D::Context> context (new Urho3D::Context());
    Urho3D::SharedPtr <Urho3D::Engine> engine (new Urho3D::Engine(context));

    context->GetSubsystem <Urho3D::Log> ()->SetLevel (Urho3D::LOG_DEBUG);
    context->RegisterFactory <CastlesStrategy::Unit> ();

    SetupEngine (engine);
    Urho3D::SharedPtr <Urho3D::Scene> scene (SetupScene (context));

    CastlesStrategy::UnitsManager unitsManager (nullptr);
    SetupUnitsManager (unitsManager, context);
    unitsManager.AddUnit (SpawnUnit (context, scene));

    const float MAX_TIME = 1000.0f;
    const float TIME_STEP = 1.0f / 60.0f;
    float elapsedTime = 0.0f;

    while (elapsedTime < MAX_TIME)
    {
        unitsManager.HandleUpdate (TIME_STEP);
        scene->Update (TIME_STEP);
        elapsedTime += TIME_STEP;
    }

    URHO3D_LOGINFO ("Result unit position: " + scene->GetChild ("TestUnitNode")->GetWorldPosition ().ToString ());
    if ((scene->GetChild ("TestUnitNode")->GetWorldPosition () - Urho3D::Vector3 (waypoints.Back ().x_, 0, waypoints.Back ().y_)).
            Length () > 1.0f)
    {
        URHO3D_LOGERROR ("Unit is to far from requested point: " + waypoints.Back ().ToString ());
        return 1;
    }
    else
    {
        return 0;
    }
}

void CustomTerminate ()
{
    try
    {
        std::rethrow_exception (std::current_exception ());
    }

    catch (UniversalException <CastlesStrategy::UnitsManager> &exception)
    {
        URHO3D_LOGERROR (exception.GetException ());
    }

    catch (UniversalException <CastlesStrategy::UnitType> &exception)
    {
        URHO3D_LOGERROR (exception.GetException ());
    }

    catch (...)
    {
        URHO3D_LOGERROR ("Unknown exception!");
    }
    abort ();
}

void SetupEngine (Urho3D::Engine *engine)
{
    Urho3D::VariantMap engineParameters;
    engineParameters [Urho3D::EP_HEADLESS] = true;
    engineParameters [Urho3D::EP_WORKER_THREADS] = false;
    engineParameters [Urho3D::EP_LOG_NAME] = "TestMovement.log";

    engineParameters [Urho3D::EP_RESOURCE_PREFIX_PATHS] = "..;.";
    engineParameters [Urho3D::EP_RESOURCE_PATHS] = "CoreData;TestData;Data";
    engine->Initialize(engineParameters);
}

Urho3D::Scene *SetupScene (Urho3D::Context *context)
{
    Urho3D::Scene *scene = new Urho3D::Scene (context);
    Urho3D::Node *planeNode = scene->CreateChild ("Plane");

    planeNode->SetPosition (Urho3D::Vector3::ZERO);
    planeNode->SetScale (100.0f);
    planeNode->CreateComponent <Urho3D::Navigable> ();

    Urho3D::ResourceCache *cache = context->GetSubsystem <Urho3D::ResourceCache> ();
    Urho3D::StaticModel *model = planeNode->CreateComponent <Urho3D::StaticModel> ();
    model->SetModel (cache->GetResource <Urho3D::Model> ("Plane.mdl"));

    Urho3D::NavigationMesh *navMesh = scene->CreateComponent <Urho3D::NavigationMesh> ();
    navMesh->Build ();
    scene->CreateComponent <Urho3D::CrowdManager> ();
    return scene;
}

void SetupUnitsManager (CastlesStrategy::UnitsManager &unitsManager, Urho3D::Context *context)
{
    Urho3D::ResourceCache *cache = context->GetSubsystem <Urho3D::ResourceCache> ();
    unitsManager.LoadUnitsTypesFromXML (cache->GetResource <Urho3D::XMLFile> ("TestUnitTypes.xml")->GetRoot ());
}

CastlesStrategy::Unit *SpawnUnit (Urho3D::Context *context, Urho3D::Scene *scene)
{
    CastlesStrategy::Unit *unit = scene->CreateChild ("TestUnitNode")->CreateComponent <CastlesStrategy::Unit> ();
    Urho3D::NavigationMesh *navMesh = scene->GetComponent <Urho3D::NavigationMesh> ();
    unit->GetNode ()->SetPosition (navMesh->FindNearestPoint (Urho3D::Vector3::ZERO));

    unit->SetOwner (0);
    unit->SetUnitType (0);
    unit->SetWaypoints (waypoints);
    return unit;
}
