add_executable(map_editor
  map_editor.cc)
target_include_directories(map_editor PRIVATE .)
target_link_libraries(map_editor PRIVATE engine sanitizers)
