add_executable(feta
  feta.cc
  init.cc
  mouse.cc
  net_receiver.cc
  net_sender.cc
)
target_include_directories(feta PRIVATE .)
target_link_libraries(feta PRIVATE thoom sanitizers)
