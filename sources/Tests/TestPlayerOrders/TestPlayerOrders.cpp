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
#include <CastlesStrategy/Server/Player/Player.hpp>

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
    context->RegisterFactory <CastlesStrategy::Unit> ();

    SetupEngine (engine);
    Urho3D::SharedPtr <Urho3D::Scene> scene (SetupScene (context));

    CastlesStrategy::ManagersHub managersHub (scene);
    CastlesStrategy::Map *map = dynamic_cast <CastlesStrategy::Map *> (managersHub.GetManager (CastlesStrategy::MI_MAP));
    SetupMap (map, context);

    CastlesStrategy::UnitsManager *unitsManager =
            dynamic_cast <CastlesStrategy::UnitsManager *> (managersHub.GetManager (CastlesStrategy::MI_UNITS_MANAGER));
    SetupUnitsManager (unitsManager, context);

    const unsigned UNIT_TYPE = 1;
    const CastlesStrategy::UnitType &unitType = unitsManager->GetUnitType (UNIT_TYPE);
    CastlesStrategy::Player player (&managersHub);
    player.AddOrder (UNIT_TYPE);

    float maxTime = unitType.GetRecruitmentTime ();
    const float TIME_STEP = 1.0f / 60.0f;
    float elapsedTime = 0.0f;

    while (elapsedTime < maxTime + TIME_STEP)
    {
        player.HandleUpdate (TIME_STEP);
        elapsedTime += TIME_STEP;
    }

    if (player.GetUnitsPullCount (UNIT_TYPE) != 0)
    {
        URHO3D_LOGERROR ("Test 1: expected 0 units in pull!");
        return 1;
    }

    const int UNITS_ORDERED = 4;
    maxTime = unitType.GetRecruitmentTime () * (UNITS_ORDERED - 1);
    elapsedTime = 0.0f;
    player.SetCoins (unitType.GetRecruitmentCost () * UNITS_ORDERED);

    for (unsigned index = 0; index < UNITS_ORDERED; index++)
    {
        player.AddOrder (UNIT_TYPE);
    }

    if (player.GetOrders ().Size () != UNITS_ORDERED)
    {
        URHO3D_LOGERROR ("Test 2: " + Urho3D::String (UNITS_ORDERED) + " units ordered, " +
            " but there is only " + Urho3D::String (player.GetOrders ().Size ()) + " units in orders!");
        return 2;
    }

    bool removeCalled = false;
    while (elapsedTime < maxTime + TIME_STEP)
    {
        player.HandleUpdate (TIME_STEP);
        if (elapsedTime >= maxTime / 2.0f && !removeCalled)
        {
            player.RemoveOrder (UNIT_TYPE);
            removeCalled = true;
        }
        elapsedTime += TIME_STEP;
    }

    if (player.GetUnitsPullCount (UNIT_TYPE) != UNITS_ORDERED - 1)
    {
        URHO3D_LOGERROR ("Test 3: expected " + Urho3D::String (UNITS_ORDERED - 1) + " units in pool, " +
                         "but there is " + Urho3D::String (player.GetUnitsPullCount (0)) + " units in pool!");
        return 3;
    }

    if (player.GetCoins () != unitType.GetRecruitmentCost ())
    {
        URHO3D_LOGERROR ("Test 4: expected " + Urho3D::String (unitType.GetRecruitmentCost ()) + " coins, " +
                         "but got " + Urho3D::String (player.GetCoins ()) + " coins!");
        return 4;
    }

    player.TakeUnitFromPull (UNIT_TYPE);
    if (player.GetUnitsPullCount (UNIT_TYPE) != UNITS_ORDERED - 2)
    {
        URHO3D_LOGERROR ("Test 5: expected " + Urho3D::String (UNITS_ORDERED - 2) + " units in pool, " +
                         "but there is " + Urho3D::String (player.GetUnitsPullCount (0)) + " units in pool!");
        return 3;
    }
    return 0;
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
    engineParameters [Urho3D::EP_LOG_NAME] = "TestPlayerOrders.log";

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
}

void SetupMap (CastlesStrategy::Map *map, Urho3D::Context *context)
{
    Urho3D::ResourceCache *cache = context->GetSubsystem <Urho3D::ResourceCache> ();
    Urho3D::XMLElement xml = cache->GetResource <Urho3D::XMLFile> ("TestMap.xml")->GetRoot ();

    map->SetSize (xml.GetIntVector2 ("size"));
    map->LoadRoutesFromXML (xml);
}
