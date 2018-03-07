#pragma once

namespace CastlesStrategy
{
enum ServerToClientNetworkMessageType
{
    STCNMT_START = 100,
    // GameStatus : Int.
    STCNMT_GAME_STATUS = 100,
    // MapPath : String.
    STCNMT_MAP_PATH,
    // ID : UInt (node id).
    STCNMT_UNIT_SPAWNED,
    STCNMT_TYPES_COUNT
};
}
