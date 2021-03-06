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

#include <Utils/UniversalException.hpp>
#include <CastlesStrategy/Shared/Unit/Unit.hpp>
#include <CastlesStrategy/Shared/Unit/UnitType.hpp>

#include <CastlesStrategy/Server/Managers/UnitsManager.hpp>
#include <CastlesStrategy/Server/Managers/Map.hpp>
#include <CastlesStrategy/Server/Managers/ManagersHub.hpp>

void CustomTerminate ();
void SetupEngine (Urho3D::Engine *engine);
Urho3D::Scene *SetupScene (Urho3D::Context *context);

void SetupUnitsManager (CastlesStrategy::UnitsManager *unitsManager, Urho3D::Context *context);
void SetupMap (CastlesStrategy::Map *map, Urho3D::Context *context);

int main (int argc, char **argv)
{
    std::set_terminate (CustomTerminate);
    Urho3D::SharedPtr <Urho3D::Context> context (new Urho3D::Context());
    Urho3D::SharedPtr <Urho3D::Engine> engine (new Urho3D::Engine(context));

    context->GetSubsystem <Urho3D::Log> ()->SetLevel (Urho3D::LOG_DEBUG);
    CastlesStrategy::Unit::RegisterObject (context);
    
    SetupEngine (engine);
    Urho3D::SharedPtr <Urho3D::Scene> scene (SetupScene (context));

    CastlesStrategy::ManagersHub managersHub (scene);
    CastlesStrategy::Map *map = dynamic_cast <CastlesStrategy::Map *> (managersHub.GetManager (CastlesStrategy::MI_MAP));
    SetupMap (map, context);

    CastlesStrategy::UnitsManager *unitsManager =
            dynamic_cast <CastlesStrategy::UnitsManager *> (managersHub.GetManager (CastlesStrategy::MI_UNITS_MANAGER));
    SetupUnitsManager (unitsManager, context);

    const float MAX_TIME = 1000.0f;
    const float TIME_STEP = 1.0f / 60.0f;
    float elapsedTime = 0.0f;

    Urho3D::SharedPtr <CastlesStrategy::Unit> spawnedUnit (unitsManager->GetUnit (
            unitsManager->SpawnUnit (0, true, 1)->GetID ()));
    Urho3D::SharedPtr <CastlesStrategy::Unit> oppositeSpawn (unitsManager->GetUnit (
            unitsManager->GetSpawn (0, false)->GetID ()));

    while (elapsedTime < MAX_TIME)
    {
        managersHub.HandleUpdate (TIME_STEP);
        scene->Update (TIME_STEP);
        elapsedTime += TIME_STEP;
    }

    URHO3D_LOGINFO ("Result unit position: " + spawnedUnit->GetNode ()->GetWorldPosition ().ToString ());
    if ((spawnedUnit->GetNode ()->GetWorldPosition () -
                Urho3D::Vector3 (map->GetWaypoint (0, 0, false).x_, 0, map->GetWaypoint (0, 0, false).y_)).
            Length () > 10.0f)
    {
        URHO3D_LOGERROR ("Unit is to far from requested point: " + map->GetWaypoint (0, 0, false).ToString ());
        return 1;
    }

    URHO3D_LOGINFO ("Result spawned unit hp: " + Urho3D::String (spawnedUnit->GetHp ()));
    URHO3D_LOGINFO ("Result opposite spawn hp: " + Urho3D::String (oppositeSpawn->GetHp ()));

    if (oppositeSpawn->GetHp () > 0)
    {
        URHO3D_LOGERROR ("Opposite spawn must be destroyed!");
        return 2;
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

    catch (AnyUniversalException &exception)
    {
        URHO3D_LOGERROR (exception.GetException ());
    }
    abort ();
}

void SetupEngine (Urho3D::Engine *engine)
{
    Urho3D::VariantMap engineParameters;
    engineParameters [Urho3D::EP_HEADLESS] = true;
    engineParameters [Urho3D::EP_WORKER_THREADS] = false;
    engineParameters [Urho3D::EP_LOG_NAME] = "TestSpawns.log";

    engineParameters [Urho3D::EP_RESOURCE_PREFIX_PATHS] = "..;.";
    engineParameters [Urho3D::EP_RESOURCE_PATHS] = "CoreData;TestData;Data";
    engine->Initialize(engineParameters);
}

Urho3D::Scene *SetupScene (Urho3D::Context *context)
{
    Urho3D::Scene *scene = new Urho3D::Scene (context);
    Urho3D::Node *planeNode = scene->CreateChild ("Plane");

    planeNode->SetPosition ({50.0f, 0.0f, 50.0f});
    planeNode->SetScale ({100.0f, 1.0f, 100.0f});
    planeNode->CreateComponent <Urho3D::Navigable> ();

    Urho3D::ResourceCache *cache = context->GetSubsystem <Urho3D::ResourceCache> ();
    Urho3D::StaticModel *model = planeNode->CreateComponent <Urho3D::StaticModel> ();
    model->SetModel (cache->GetResource <Urho3D::Model> ("Plane.mdl"));

    Urho3D::NavigationMesh *navMesh = scene->CreateComponent <Urho3D::NavigationMesh> ();
    navMesh->Build ();
    scene->CreateComponent <Urho3D::CrowdManager> ();
    return scene;
}

void SetupUnitsManager (CastlesStrategy::UnitsManager *unitsManager, Urho3D::Context *context)
{
    Urho3D::ResourceCache *cache = context->GetSubsystem <Urho3D::ResourceCache> ();
    unitsManager->LoadUnitsTypesFromXML (cache->GetResource <Urho3D::XMLFile> ("TestUnitTypes.xml")->GetRoot ());
    unitsManager->LoadSpawnsFromXML (cache->GetResource <Urho3D::XMLFile> ("TestMap.xml")->GetRoot ());
}

void SetupMap (CastlesStrategy::Map *map, Urho3D::Context *context)
{
    Urho3D::ResourceCache *cache = context->GetSubsystem <Urho3D::ResourceCache> ();
    Urho3D::XMLElement xml = cache->GetResource <Urho3D::XMLFile> ("TestMap.xml")->GetRoot ();

    map->SetSize (xml.GetIntVector2 ("size"));
    map->LoadRoutesFromXML (xml);
}
