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
#include <CastlesStrategy/Shared/Village/Village.hpp>

#include <CastlesStrategy/Server/Managers/VillagesManager.hpp>
#include <CastlesStrategy/Server/Managers/UnitsManager.hpp>
#include <CastlesStrategy/Server/Managers/PlayersManager.hpp>
#include <CastlesStrategy/Server/Managers/Map.hpp>
#include <CastlesStrategy/Server/Managers/ManagersHub.hpp>

void CustomTerminate ();
void SetupEngine (Urho3D::Engine *engine);
Urho3D::Scene *SetupScene (Urho3D::Context *context);

void SetupUnitsManager (CastlesStrategy::UnitsManager *unitsManager, Urho3D::Context *context);
void SetupMap (CastlesStrategy::Map *map, Urho3D::Context *context);
void SetupVillagesManager (CastlesStrategy::VillagesManager *villagesManager, Urho3D::Context *context);

int main (int argc, char **argv)
{
    std::set_terminate (CustomTerminate);
    Urho3D::SharedPtr <Urho3D::Context> context (new Urho3D::Context());
    Urho3D::SharedPtr <Urho3D::Engine> engine (new Urho3D::Engine(context));

    context->GetSubsystem <Urho3D::Log> ()->SetLevel (Urho3D::LOG_DEBUG);
    CastlesStrategy::Unit::RegisterObject (context);
    CastlesStrategy::Village::RegisterObject (context);

    SetupEngine (engine);
    Urho3D::SharedPtr <Urho3D::Scene> scene (SetupScene (context));

    CastlesStrategy::ManagersHub managersHub (scene);
    CastlesStrategy::Map *map = dynamic_cast <CastlesStrategy::Map *> (managersHub.GetManager (CastlesStrategy::MI_MAP));
    SetupMap (map, context);

    CastlesStrategy::UnitsManager *unitsManager =
            dynamic_cast <CastlesStrategy::UnitsManager *> (managersHub.GetManager (CastlesStrategy::MI_UNITS_MANAGER));
    SetupUnitsManager (unitsManager, context);

    CastlesStrategy::PlayersManager *playersManager =
            dynamic_cast <CastlesStrategy::PlayersManager *> (managersHub.GetManager (CastlesStrategy::MI_PLAYERS_MANAGER));
    playersManager->SetFirstPlayer (CastlesStrategy::Player (&managersHub));
    playersManager->SetSecondPlayer (CastlesStrategy::Player (&managersHub));

    CastlesStrategy::VillagesManager *villagesManager =
            dynamic_cast <CastlesStrategy::VillagesManager *> (managersHub.GetManager (CastlesStrategy::MI_VILLAGES_MANAGER));
    SetupVillagesManager (villagesManager, context);

    const float MAX_TIME = 2.1f;
    const float TIME_STEP = 1.0f / 60.0f;
    float elapsedTime = 0.0f;

    while (elapsedTime < MAX_TIME)
    {
        managersHub.HandleUpdate (TIME_STEP);
        scene->Update (TIME_STEP);
        elapsedTime += TIME_STEP;
    }

    URHO3D_LOGINFO ("Result first player coins: " + Urho3D::String (playersManager->GetFirstPlayer ().GetCoins ()));
    URHO3D_LOGINFO ("Result second player coins: " + Urho3D::String (playersManager->GetSecondPlayer ().GetCoins ()));

    unsigned int firstCoinsExpected = static_cast <unsigned int> (
            trunc (MAX_TIME / CastlesStrategy::DEFAULT_TAXES_DELAY) * 2000 * CastlesStrategy::OWNERSHIP_TO_MONEY_PER_SECOND);

    unsigned int secondCoinsExpected = static_cast <unsigned int> (
            trunc (MAX_TIME / CastlesStrategy::DEFAULT_TAXES_DELAY) * 3000 * CastlesStrategy::OWNERSHIP_TO_MONEY_PER_SECOND);

    if (playersManager->GetFirstPlayer ().GetCoins () != firstCoinsExpected)
    {
        URHO3D_LOGERROR ("First player coins expectancy is " + Urho3D::String (firstCoinsExpected) + "!");
        return 1;
    }

    else if (playersManager->GetSecondPlayer ().GetCoins () != secondCoinsExpected)
    {
        URHO3D_LOGERROR ("Second player coins expectancy is " + Urho3D::String (secondCoinsExpected) + "!");
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
    engineParameters [Urho3D::EP_LOG_NAME] = "TestVillages.log";

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

void SetupVillagesManager (CastlesStrategy::VillagesManager *villagesManager, Urho3D::Context *context)
{
    Urho3D::ResourceCache *cache = context->GetSubsystem <Urho3D::ResourceCache> ();
    Urho3D::XMLElement xml = cache->GetResource <Urho3D::XMLFile> ("TestMap.xml")->GetRoot ();
    villagesManager->LoadVillagesFromXML (xml);
}
