add_library(shared
  main.cc
  render_functions.cc
  sounds.cc
  hud.cc
  ui_box.cc
  icon.cc
)

target_include_directories(shared PUBLIC .)
target_link_libraries(shared PUBLIC thoom)
