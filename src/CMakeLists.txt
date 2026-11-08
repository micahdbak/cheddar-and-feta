add_subdirectory(foes)
add_subdirectory(items)

add_executable(cnf
  billboard.cc
  cheddar.cc
  credits.cc
  freedom.cc
  game.cc
  game_hud.cc
  game_over.cc
  guest_feta.cc
  guest_game.cc
  guest_peer_receiver.cc
  guest_peer_sender.cc
  hitbox.cc
  host_game.cc
  host_peer_receiver.cc
  host_peer_sender.cc
  hurtbox.cc
  icon.cc
  init.cc
  init_host.cc
  init_join.cc
  ladder.cc
  main.cc
  mouse.cc
  remote_feta.cc
  render_functions.cc
  sounds.cc
  splash.cc
  textbox.cc)

target_include_directories(cnf PRIVATE .)
target_link_libraries(cnf PRIVATE cnf_foes cnf_items thoom sanitizers)

if(CNF_ONSCREEN_DEBUG)
  target_compile_definitions(cnf PRIVATE ONSCREEN_DEBUG)
endif()
