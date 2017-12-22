set (TARGET_NAME CastlesStrategyClient)
define_source_files (RECURSE GLOB_H_PATTERNS "*.hpp")
define_dependency_libs (Urho3D)
setup_library (SHARED)
