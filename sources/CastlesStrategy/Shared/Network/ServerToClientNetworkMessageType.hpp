#pragma once

namespace CastlesStrategy
{
enum ServerToClientNetworkMessageType
{
    // GameStatus : Int.
    STCNMT_GAME_STATUS = 0,
    // UnitsTypesXML : String.
    STCNMT_UNITS_TYPES_XML,
    STCNMT_TYPES_COUNT
};
}
