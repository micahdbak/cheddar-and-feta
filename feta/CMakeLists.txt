add_executable(feta
  main.cc
  feta.cc
  mouse.cc
  peer_receiver.cc
  peer_sender.cc
)
target_include_directories(feta PRIVATE .)
target_link_libraries(feta PRIVATE shared sanitizers)
