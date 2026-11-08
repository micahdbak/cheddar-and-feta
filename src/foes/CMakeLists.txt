add_library(cnf_foes OBJECT
  bat.cc
  bell.cc
  bug.cc
  foe.cc
  foe_dijkstra.cc
  frog.cc
  gate.cc
  porcupine.cc
  spawner.cc
  spitter/abdomen.cc
  spitter/segment.cc
  spitter/spitter.cc)

target_include_directories(cnf_foes PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(cnf_foes PRIVATE thoom sanitizers)
