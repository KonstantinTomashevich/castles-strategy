#pragma once

namespace CastlesStrategy
{
enum ServerToClientNetworkMessageType
{
    STCNMT_START = 100,
    // GameStatus : Int.
    STCNMT_GAME_STATUS = 100,
    // MapPath : String.
    STCNMT_INITIAL_INFO,
    // PlayerType : byte.
    STCNMT_PLAYER_TYPE,
    // ID : UInt (node id).
    STCNMT_UNIT_SPAWNED,
    // UnitType : UInt, NewValue : UInt.
    STCNMT_UNITS_PULL_SYNC,
    // NewValue : UInt.
    STCNMT_COINS_SYNC,
    // Message : String.
    STCNMT_CHAT_MESSAGE,
    STCNMT_TYPES_COUNT
};
}
