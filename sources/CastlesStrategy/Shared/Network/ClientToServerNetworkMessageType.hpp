#pragma once

namespace CastlesStrategy
{
enum ClientToServerNetworkMessageType
{
    CTSNMT_START = 200,
    // UnitType : UInt.
    CTSNMT_ADD_ORDER = 200,
    // SpawnID : UInt, UnitType : UInt.
    CTSNMT_SPAWN_UNIT,
    // Message : String.
    CTSNMT_CHAT_MESSAGE,
    //
    CTSNMT_REQUEST_TO_BE_A_PLAYER,
    //
    CTSNMT_REQUEST_TO_BE_AN_OBSERVER,
    // IsReady : bool.
    CTSNMT_SET_IS_READY_FOR_START,
    CTSNMT_TYPES_COUNT
};
}
