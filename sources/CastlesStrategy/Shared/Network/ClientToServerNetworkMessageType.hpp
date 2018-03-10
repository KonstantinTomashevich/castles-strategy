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
    CTSNMT_TYPES_COUNT
};
}
