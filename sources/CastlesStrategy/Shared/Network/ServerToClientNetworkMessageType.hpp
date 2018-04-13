#pragma once

namespace CastlesStrategy
{
enum ServerToClientNetworkMessageType
{
    STCNMT_START = 100,
    // GameStatus : Int.
    STCNMT_GAME_STATUS = 100,
    // ID : UInt (node id).
    STCNMT_OBJECT_SPAWNED,
    // UnitType : UInt, NewValue : UInt.
    STCNMT_UNITS_PULL_SYNC,
    // NewValue : UInt.
    STCNMT_COINS_SYNC,
    // Message : String.
    STCNMT_CHAT_MESSAGE,
    // PlayerName : String, PlayerType : UByte, ReadyForStart : Bool.
    STCNMT_NEW_PLAYER,
    // PlayerName : String, PlayerType : UByte.
    STCNMT_PLAYER_TYPE_CHANGED,
    // PlayerName : String, ReadyForStart : Bool.
    STCNMT_PLAYER_READY_CHANGED,
    // PlayerName : String.
    STCNMT_PLAYER_LEFT,
    // MapName : String, ResourcesCount : UInt, (Name : String, Bytes : PODVector <UByte>) x ResourcesCount.
    STCNMT_MAP_FILES,
    STCNMT_TYPES_COUNT
};
}
