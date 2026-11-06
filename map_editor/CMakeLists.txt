add_executable(map_editor
  main.cc
  editor.cc
  editor_ui.cc)
target_include_directories(map_editor PRIVATE .)
target_link_libraries(map_editor PRIVATE shared sanitizers)
