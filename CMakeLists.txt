# Set CMake minimum version and CMake policy required by UrhoCommon module
cmake_minimum_required (VERSION 3.2.3)
if (COMMAND cmake_policy)
    # Libraries linked via full path no longer produce linker search paths
    cmake_policy (SET CMP0003 NEW)
    # INTERFACE_LINK_LIBRARIES defines the link interface
    cmake_policy (SET CMP0022 NEW)
    # Disallow use of the LOCATION target property - so we set to OLD as we still need it
    cmake_policy (SET CMP0026 OLD)
    # MACOSX_RPATH is enabled by default
    cmake_policy (SET CMP0042 NEW)
    # Honor the visibility properties for SHARED target types only
    cmake_policy (SET CMP0063 OLD)
endif ()
project (CastlesStrategy)

# Copy Urho3D CMake modules.
if (NOT CMAKE_MODULE_PATH)
    message (STATUS "CMAKE_MODULE_PATH isn't setted! Setting CMAKE_MODULE_PATH to urho3d-cmake/Modules.")
    set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/urho3d-cmake/Modules")
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/urho3d-cmake/")
        message (STATUS "Copying Urho3D's CMake modules...")
        if (NOT EXISTS "$ENV{URHO3D_HOME}/share/CMake/")
            file (COPY "$ENV{URHO3D_HOME}/share/Urho3D/CMake/" DESTINATION "${CMAKE_SOURCE_DIR}/urho3d-cmake/")
        else ()
            file (COPY "$ENV{URHO3D_HOME}/share/CMake/" DESTINATION "${CMAKE_SOURCE_DIR}/urho3d-cmake")
        endif ()
    endif ()
    message (STATUS "Setted CMAKE_MODULE_PATH to urho3d-cmake/Modules!")
endif ()

# Copy Urho3D CoreData.
if (NOT EXISTS "${CMAKE_SOURCE_DIR}/bin/CoreData")
    message (STATUS "Copying Urho3D's CoreData asset...")
    if (NOT EXISTS "$ENV{URHO3D_HOME}/share/Resources/CoreData/")
        file (COPY "$ENV{URHO3D_HOME}/share/Urho3D/Resources/CoreData/" DESTINATION "${CMAKE_SOURCE_DIR}/bin/CoreData")
    else ()
        file (COPY "$ENV{URHO3D_HOME}/share/Resources/CoreData/" DESTINATION "${CMAKE_SOURCE_DIR}/bin/CoreData")
    endif ()
endif ()

# Copy Urho3DEditorData.
if (NOT EXISTS "${CMAKE_SOURCE_DIR}/bin/Urho3DEditorData")
    message (STATUS "Copying Urho3D's Data asset (for editor)...")
    if (NOT EXISTS "$ENV{URHO3D_HOME}/share/Resources/Data/")
        file (COPY "$ENV{URHO3D_HOME}/share/Urho3D/Resources/Data/" DESTINATION "${CMAKE_SOURCE_DIR}/bin/Urho3DEditorData")
    else ()
        file (COPY "$ENV{URHO3D_HOME}/share/Resources/Data/" DESTINATION "${CMAKE_SOURCE_DIR}/bin/Urho3DEditorData")
    endif ()
endif ()

if (NOT EXISTS "${CMAKE_BINARY_DIR}/bin/Urho3DEditorData")
    message (STATUS "Copying Urho3D's Data asset (for editor) to binary dir...")
    file (COPY "${CMAKE_SOURCE_DIR}/bin/Urho3DEditorData/" DESTINATION "${CMAKE_BINARY_DIR}/bin/Urho3DEditorData")
endif ()

include (UrhoCommon)
find_package (Urho3D REQUIRED)
include_directories (${URHO3D_INCLUDE_DIRS})

option (CASTLES_STRATEGY_ENABLE_TESTS "Includes test targets (for CTest)." 0)
if (CASTLES_STRATEGY_ENABLE_TESTS)
    enable_testing ()
endif ()
add_subdirectory (sources)
