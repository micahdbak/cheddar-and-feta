add_library(cnf_items OBJECT
  cannon_ball.cc
  cheese.cc
  fire.cc
  item.cc
  molotov.cc
  persister.cc
  toothpick.cc
  tossed.cc)

target_include_directories(cnf_items PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(cnf_items PRIVATE thoom sanitizers)
