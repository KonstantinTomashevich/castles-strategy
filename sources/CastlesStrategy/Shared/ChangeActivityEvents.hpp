#pragma once
#include <Urho3D/Core/Object.h>

namespace CastlesStrategy
{
URHO3D_EVENT (SHUTDOWN_ALL_ACTIVITIES, ShutdownAllActivities)
{

}

URHO3D_EVENT (START_MAIN_MENU, StartMainMenu)
{

}

URHO3D_EVENT (START_CLIENT, StartClient)
{
    URHO3D_PARAM (PLAYER_NAME, PlayerName);
    URHO3D_PARAM (ADDRESS, Address);
    URHO3D_PARAM (PORT, Port);
}

URHO3D_EVENT (START_SERVER, StartServer)
{

}
}
