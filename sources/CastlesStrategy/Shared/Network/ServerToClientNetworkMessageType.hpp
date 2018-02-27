#pragma once

namespace CastlesStrategy
{
enum ServerToClientNetworkMessageType
{
    STCNMT_START = 100,
    // GameStatus : Int.
    STCNMT_GAME_STATUS = 100,
    // UnitsTypesXML : String.
    STCNMT_UNITS_TYPES_XML,
    STCNMT_TYPES_COUNT
};
}
