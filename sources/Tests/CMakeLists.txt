macro(setup_test_executable ARG_NAME)
    set (TARGET_NAME "${ARG_NAME}")
    define_source_files (RECURSE GLOB_H_PATTERNS *.hpp)
    set (RESOURCE_DIRS "${CMAKE_SOURCE_DIR}/bin/CoreData;${CMAKE_SOURCE_DIR}/bin/CoreData;${CMAKE_SOURCE_DIR}/bin/TestData")
    setup_executable ()

    target_link_libraries (${ARG_NAME} CastlesStrategyServer)
    add_test (NAME ${ARG_NAME} COMMAND ${ARG_NAME})
    set_target_properties (
            ${ARG_NAME}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/Tests"
    )
endmacro ()

add_subdirectory (TestMovement)
add_subdirectory (TestAttackAndMovement)
