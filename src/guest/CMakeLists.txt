add_executable(feta
  main.cc
  guest_feta.cc
  guest_mouse.cc
  guest_peer_receiver.cc
  guest_peer_sender.cc
  ../launch.cc
)
target_include_directories(feta PRIVATE . ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(feta PRIVATE shared sanitizers)
