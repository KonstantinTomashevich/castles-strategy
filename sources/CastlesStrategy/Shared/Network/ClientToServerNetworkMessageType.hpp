#pragma once

namespace CastlesStrategy
{
enum ClientToServerNetworkMessageType
{
    // UnitType : UInt.
    CTSNMT_ADD_ORDER = 0,
    // SpawnerIndex : UInt, UnitType : UInt.
    CTSNMT_SPAWN_UNIT,
    CTSNMT_TYPES_COUNT
};
}
