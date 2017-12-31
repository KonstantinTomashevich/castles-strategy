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

CastlesStrategy::Unit *SpawnFirstUnit (Urho3D::Context *context, Urho3D::Scene *scene);
CastlesStrategy::Unit *SpawnSecondUnit (Urho3D::Context *context, Urho3D::Scene *scene);

const Urho3D::PODVector <Urho3D::Vector2> firstWaypoints = {{20.0f, 20.0f}, {0.0f, 40.0f}};
const Urho3D::PODVector <Urho3D::Vector2> secondWaypoints = {{20.0f, 20.0f}};

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

    Urho3D::SharedPtr <CastlesStrategy::Unit> firstUnit (SpawnFirstUnit (context, scene));
    Urho3D::SharedPtr <CastlesStrategy::Unit> secondUnit (SpawnSecondUnit (context, scene));

    unitsManager.AddUnit (firstUnit);
    unitsManager.AddUnit (secondUnit);

    const float MAX_TIME = 1000.0f;
    const float TIME_STEP = 1.0f / 60.0f;
    float elapsedTime = 0.0f;

    while (elapsedTime < MAX_TIME)
    {
        unitsManager.HandleUpdate (TIME_STEP);
        scene->Update (TIME_STEP);
        elapsedTime += TIME_STEP;
    }

    URHO3D_LOGINFO ("Result first unit health: " + Urho3D::String (firstUnit->GetHp ()));
    URHO3D_LOGINFO ("Result second unit health: " + Urho3D::String (secondUnit->GetHp ()));

    if (secondUnit->GetHp () != 0)
    {
        URHO3D_LOGERROR ("Second unit must dead!");
        return 1;
    }

    URHO3D_LOGINFO ("Result first unit position: " + firstUnit->GetNode ()->GetWorldPosition ().ToString ());
    if ((firstUnit->GetNode ()->GetWorldPosition () - Urho3D::Vector3 (firstWaypoints.Back ().x_, 0, firstWaypoints.Back ().y_)).
            Length () > 1.0f)
    {
        URHO3D_LOGERROR ("First unit is to far from requested point: " + firstWaypoints.Back ().ToString ());
        return 2;
    }

    return 0;
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
    engineParameters [Urho3D::EP_LOG_NAME] = "TestAttackAndMovement.log";

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
    try
    {
        unitsManager.LoadUnitsTypesFromXML (cache->GetResource <Urho3D::XMLFile> ("TestUnitTypes.xml")->GetRoot ());
    }
    catch (UniversalException <CastlesStrategy::UnitType> &exception)
    {
        URHO3D_LOGERROR (exception.GetException ());
        abort ();
    }
}

CastlesStrategy::Unit *SpawnFirstUnit (Urho3D::Context *context, Urho3D::Scene *scene)
{
    CastlesStrategy::Unit *unit = scene->CreateChild ("FirstUnitNode")->CreateComponent <CastlesStrategy::Unit> ();
    Urho3D::NavigationMesh *navMesh = scene->GetComponent <Urho3D::NavigationMesh> ();
    unit->GetNode ()->SetPosition (navMesh->FindNearestPoint (Urho3D::Vector3::ZERO));

    unit->SetOwner (0);
    unit->SetUnitType (0);
    unit->SetWaypoints (firstWaypoints);
    return unit;
}

CastlesStrategy::Unit *SpawnSecondUnit (Urho3D::Context *context, Urho3D::Scene *scene)
{
    CastlesStrategy::Unit *unit = scene->CreateChild ("SecondUnitNode")->CreateComponent <CastlesStrategy::Unit> ();
    Urho3D::NavigationMesh *navMesh = scene->GetComponent <Urho3D::NavigationMesh> ();
    unit->GetNode ()->SetPosition (navMesh->FindNearestPoint (Urho3D::Vector3 (
            secondWaypoints.Front ().x_, 0.0f, secondWaypoints.Front ().y_)));

    unit->SetOwner (1);
    unit->SetUnitType (1);
    unit->SetWaypoints (secondWaypoints);
    return unit;
}
