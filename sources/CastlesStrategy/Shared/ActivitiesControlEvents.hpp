#pragma once
#include <Urho3D/Core/Object.h>

namespace CastlesStrategy
{
URHO3D_EVENT (E_SHUTDOWN_ALL_ACTIVITIES, ShutdownAllActivities)
{

}

URHO3D_EVENT (E_START_MAIN_MENU, StartMainMenu)
{

}

URHO3D_EVENT (E_START_CLIENT, StartClient)
{
    URHO3D_PARAM (PLAYER_NAME, PlayerName);
    URHO3D_PARAM (ADDRESS, Address);
    URHO3D_PARAM (PORT, Port);
}

URHO3D_EVENT (E_START_SERVER, StartServer)
{

}

URHO3D_EVENT (E_REQUEST_GAME_START, RequestGameStart)
{

}

URHO3D_EVENT (E_REQUEST_KICK_PLAYER, RequestKickPlayer)
{
    URHO3D_PARAM (PLAYER_NAME, PlayerName);
}
}
