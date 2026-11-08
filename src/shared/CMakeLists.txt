add_library(shared
  game.cc
  render_functions.cc
  sounds.cc
  game_hud.cc
  icon.cc
)

target_include_directories(shared PUBLIC .)
target_link_libraries(shared PUBLIC thoom)
