include_directories (${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory (ActivitiesApplication)
add_subdirectory (CastlesStrategyServer)
add_subdirectory (CastlesStrategyClient)
add_subdirectory (CastlesStrategyLauncher)

if (CASTLES_STRATEGY_ENABLE_TESTS)
    add_subdirectory (Tests)
endif ()
