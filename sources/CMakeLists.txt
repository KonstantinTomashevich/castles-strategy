include_directories (${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory (CastlesStrategyServer)
add_subdirectory (CastlesStrategyLauncher)

if (DUTCHY_ENABLE_TESTS)
    add_subdirectory (Tests)
endif ()
